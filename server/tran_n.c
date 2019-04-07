#include"tranfile.h"

extern int uploadNumberOfCharactor;

int SendCycle(int fd,char* sendFile,int sendLen)
{
    int sendTotal = 0;
    int ret;
    while(sendTotal<sendLen)
    {
        ret = send(fd,sendFile+sendTotal,sendLen-sendTotal,0);
        if(-1 == ret)
        {
            return -1;
        }
        sendTotal += ret;
        uploadNumberOfCharactor +=ret;
        printf("%d\r",sendTotal);
        fflush(stdout);
    }
    return 0;
}
int RecvCycle(int fd,char* recvFile,int recvLen)
{
    int recvTotal = 0;
    int ret;
    while(recvTotal<recvLen)
    {
        ret = recv(fd,recvFile+recvTotal,recvLen-recvTotal,0);
        if(0 == ret)
        {
            return -1;
        }
        recvTotal += ret;
    }
    return 0;
}
int RecvCycBig(int fd,char* recvFile,int recvLen,int recvtotl,int filesize)
{
    int recvTotal = 0;
    int ret;
    while(recvTotal<recvLen)
    {
        ret = recv(fd,recvFile+recvTotal,recvLen-recvTotal,0);
        if(0  == ret)
        {
            return -1;
        }
        recvTotal +=ret;
        recvtotl += ret;
        printf("%5.2f%s\r",(recvTotal*1.0)/filesize,"%");
        fflush(stdout);
    }
    return 0;
}
