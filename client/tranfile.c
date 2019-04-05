#include"tranfile.h"

int RecvFile(int sockfd)
{
    int ret = 0;
    int size = 0;
    off_t filesize = 0;
    off_t realsize = 0;
    int bufsize=0;
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
    ret = RecvCycle(sockfd,(char*)&filesize,size);
    RETURN_MINUSONE(-1,ret,"RecvCycle");
    //recv file data
    printf("filename:%s,filesize:%ld\n",filename,filesize);
    int fd = open(filename,O_RDWR|O_CREAT,0666);
    while(1)
    {
        ret = RecvCycle(sockfd,(char*)&size,sizeof(int));
        if(size <= 0)
        {
            printf("100.00%s\n","%");
            break;
        }
        ret = RecvCycle(sockfd,buf,size);
        RETURN_MINUSONE(-1,ret,"RecvCycle");
        write(fd,buf,size);
        realsize +=size;
        if(((realsize*1.0/filesize)*100-bufsize)>1)
        {
            bufsize++;
            printf("%5.2f%s\r",(realsize*1.0/filesize)*100,"%");
            fflush(stdout);
        }
    }
    printf("Recv Success!\n");
    return 0;
}

int SendFile(int sockfd,char* filename)
{
    train t;
    int ret;
    off_t realsize = 0,filesize;
    int bufsize = 0;
    memset(&t,0,sizeof(train));
    strcpy(t.buf,filename);
    t.dataLen = strlen(filename);
    ret = SendCycle(sockfd,(char *)&t,4+t.dataLen);
    RETURN_MINUSONE(-1,ret,"SendCycle");
    int fd = open(filename,O_RDONLY);
    struct stat buf;
    ret = fstat(fd,&buf);
    filesize = buf.st_size;
    RETURN_MINUSONE(-1,ret,"fstat");
    t.dataLen = sizeof(buf.st_size);
    memcpy(t.buf,&buf.st_size,sizeof(buf.st_size));
    ret = SendCycle(sockfd,(char *)&t,4+t.dataLen);
    RETURN_MINUSONE(-1,ret,"SendCycle");
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
    printf("100.00%s\n","%");
    ret = SendCycle(sockfd,(char *)&t,4);
    RETURN_MINUSONE(-1,ret,"SendCycle");
    printf("Send Success!\n");
    close(fd);
    return 0;
}
