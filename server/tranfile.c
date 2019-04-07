#include"tranfile.h"
extern char uploadNameOfFile[64];
int RecvFile(int sockfd,char* fn)
{
    int fd;
    int uploadtmp;
    int opseat;
    int ret = 0;
    int size = 0;
    off_t filesize = 0;
    char uploadFlag;
    char sizebuf[64]={0};
    char filename[50] = {0};
    char buf[1000] = {0};
    //recv file name
    ret = RecvCycle(sockfd,(char *)&size,sizeof(int));
    RETURN_MINUSONE(-1,ret,"RecvCycle");
    ret = RecvCycle(sockfd,filename,size);
    RETURN_MINUSONE(-1,ret,"RecvCycle");
    //recv file size
    ret = RecvCycle(sockfd,(char*)&size,sizeof(int));
    RETURN_MINUSONE(-1,ret,"RecvCycle");
    ret = RecvCycle(sockfd,sizebuf,size);
    uploadFlag = sizebuf[strlen(sizebuf)-1];
    sizebuf[strlen(sizebuf)-1] = '\0';
    filesize = atoi(sizebuf);
    RETURN_MINUSONE(-1,ret,"RecvCycle");
    //recv file data
    printf("filename:%s,filesize:%ld\n",filename,filesize);
    if(uploadFlag=='1')
    {
        if(filesize < 104857600)
        {
            fd = open(fn,O_RDWR|O_APPEND);
            struct stat ss;
            fstat(fd,&ss);
            size = ss.st_size;
            SendCycle(sockfd,(char*)&size,sizeof(int));
            while(1)
            {
                ret = RecvCycle(sockfd,(char*)&size,sizeof(int));
                if(size <= 0)
                {
                    break;
                }
                ret = RecvCycle(sockfd,buf,size);
                RETURN_MINUSONE(-1,ret,"RecvCycle");
                write(fd,buf,size);
            }
            close(fd);
            printf("Recv Success!\n");
        }
        else
        {
            fd = open(fn,O_RDWR);
            struct stat ss;
            fstat(fd,&ss);
            opseat = (ss.st_blocks-1)/8;
            uploadtmp = filesize - (opseat*4096);
            printf("tmp:%d|op:%d|file:%ld\n",uploadtmp,opseat,filesize);
            SendCycle(sockfd,(char*)&opseat,sizeof(int));
            char* pMmap;
            pMmap = (char*)mmap(NULL,filesize,PROT_WRITE|PROT_READ,MAP_SHARED,fd,opseat*4096);
            ret = RecvCycBig(sockfd,pMmap,uploadtmp,opseat*4096,filesize);
            printf("opseat:%d\n",opseat);
            if(-1 == ret)
            {
                return -2;
            }
            munmap(pMmap,filesize);
            ret = RecvCycle(sockfd,(char*)&size,sizeof(int));
            printf("Recv Success!\n");
            close(fd);
        }

    }
    else
    {
        if(filesize < 104857600)
        {
            fd = open(fn,O_RDWR|O_CREAT,0666);
            while(1)
            {
                ret = RecvCycle(sockfd,(char*)&size,sizeof(int));
                if(-1 == ret)
                {
                    return -2;
                }
                if(size <= 0)
                {
                    break;
                }
                ret = RecvCycle(sockfd,buf,size);
                RETURN_MINUSONE(-1,ret,"RecvCycle");
                write(fd,buf,size);
            }
            close(fd);
            printf("Recv Success!\n");
        }
        else
        {
            uploadtmp = filesize;
            opseat = 0;
            char* mp;
            int fd = open(fn,O_RDWR|O_CREAT,0666);
            ret = ftruncate(fd,filesize);
            if(-1 == ret)
            {
                perror("ftruncate");
            }
            mp = (char*)mmap(NULL,filesize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
            ret = RecvCycBig(sockfd,mp,uploadtmp,opseat,filesize);
            if(-1 == ret)
            {
                return -2;
            }
            munmap(mp,filesize);
            ret = RecvCycle(sockfd,(char*)&size,sizeof(int));
            close(fd);
        }
    }
    return 0;
}

int SendFile(int sockfd,char* filename,char* realname)
{
    printf("Start translation File!\n");
    train t;
    char uploadFlag[2]="0";
    int uploadsize=0;
    char upcharnum[64]={0};
    int ret;
    off_t realsize = 0,filesize;
    int bufsize = 0;
    memset(&t,0,sizeof(train));
    memset(uploadNameOfFile,0,sizeof(uploadNameOfFile));
    strcpy(uploadNameOfFile,filename);
    strcpy(t.buf,filename);
    t.dataLen = strlen(filename);
    ret = SendCycle(sockfd,(char *)&t,4+t.dataLen);
    RETURN_MINUSONE(-1,ret,"SendCycle");

    int fd = open(realname,O_RDWR);
    //get upload date;
    sprintf(uploadNameOfFile,"%s_uploadLog",uploadNameOfFile);
    int fd2 = open(uploadNameOfFile,O_RDONLY|O_CREAT,0600);
    ret = read(fd2,upcharnum,sizeof(upcharnum));
    if(ret)
    {
        strcpy(uploadFlag,"1");
    }
    close(fd2);
    struct stat buf;
    ret = fstat(fd,&buf);
    RETURN_MINUSONE(-1,ret,"fstat");
    filesize = buf.st_size;
    sprintf(t.buf,"%ld%s",buf.st_size,uploadFlag);
    t.dataLen = sizeof(t.buf);
    ret = SendCycle(sockfd,(char *)&t,4+t.dataLen);
    RETURN_MINUSONE(-1,ret,"SendCycle");
    if(!strcmp(uploadFlag,"1"))
    {
        RecvCycle(sockfd,(char*)&uploadsize,sizeof(int));
    }
    if(filesize < 104857600)
    {
        lseek(fd,uploadsize,SEEK_SET);
        printf("upsize:%d\n",uploadsize);
        while((t.dataLen = read(fd,t.buf,sizeof(t.buf)))>0)
        {
            ret = SendCycle(sockfd,(char*)&t,4+t.dataLen);
            RETURN_MINUSONE(-1,ret,"SendCycle");
            realsize +=t.dataLen;
            if(((realsize*1.0/filesize)*100-bufsize)>1)
            {
                bufsize++;
                printf("%5.2f%s\r",(realsize*1.0/filesize)*100,"%");
                fflush(stdout);
            }
        }
        ret = SendCycle(sockfd,(char *)&t,4);
    }
    else
    {
        char* pMmap;
        pMmap = (char*)mmap(NULL,filesize,PROT_READ,MAP_SHARED,fd,uploadsize*4096);
        printf("ss:%d\n",uploadsize);
        filesize = filesize-(uploadsize*4096);
        ret = SendCycle(sockfd,pMmap,filesize);
        t.dataLen = 0;
        ret = SendCycle(sockfd,(char*)&t.dataLen,4);
        munmap(pMmap,filesize);
    }
    printf("100.00%s\n","%");
    RETURN_MINUSONE(-1,ret,"SendCycle");
    printf("Send Success!\n");
    close(fd);
    return 0;
}
