#include"tranfile.h"

int RecvFile(int sockfd,char* fn)
{
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
    puts(sizebuf);
    printf("flag:%c\n",uploadFlag);
    RETURN_MINUSONE(-1,ret,"RecvCycle");
    //recv file data
    printf("filename:%s,filesize:%ld\n",filename,filesize);
    if(uploadFlag=='1')
    {
        int fd = open(fn,O_RDWR|O_APPEND);
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
        printf("Recv Success!\n");
    }
    else
    {
        int fd = open(fn,O_RDWR|O_CREAT,0666);
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
        printf("Recv Success!\n");
    }
    return 0;
}

int SendFile(int sockfd,char* filename,char* realname)
{
    train t;
    int ret;
    memset(&t,0,sizeof(train));
    strcpy(t.buf,filename);
    t.dataLen = strlen(filename);
    ret = SendCycle(sockfd,(char *)&t,4+t.dataLen);
    RETURN_MINUSONE(-1,ret,"SendCycle");
    int fd = open(realname,O_RDONLY);
    struct stat buf;
    ret = fstat(fd,&buf);
    RETURN_MINUSONE(-1,ret,"fstat");
    t.dataLen = sizeof(buf.st_size);
    memcpy(t.buf,&buf.st_size,sizeof(buf.st_size));
    ret = SendCycle(sockfd,(char *)&t,4+t.dataLen);
    RETURN_MINUSONE(-1,ret,"SendCycle");
    while((t.dataLen = read(fd,t.buf,sizeof(t.buf)))>0)
    {
        ret = SendCycle(sockfd,(char*)&t,4+t.dataLen);
        RETURN_MINUSONE(-1,ret,"SendCycle");
    }
    ret = SendCycle(sockfd,(char *)&t,4);
    RETURN_MINUSONE(-1,ret,"SendCycle");
    printf("Send Success!\n");
    return 0;
}
