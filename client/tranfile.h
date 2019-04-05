#ifndef __TRAINFILE_H__
#define __TRAINFILE_H__
#include"head.h"
typedef struct
{
    int dataLen;
    char buf[1000];
}train;

int SendCycle(int,char*,int);
int RecvCycle(int,char*,int);
int RecvFile(int);
int SendFile(int,char*);
int TcpInitClient(int*,char*,char*);

#define FILENAME "file"

#endif
