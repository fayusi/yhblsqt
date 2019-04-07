#ifndef __TRAINFILE_H__
#define __TRAINFILE_H__
#include"head.h"
typedef struct
{
    int dataLen;
    char buf[1000];
}train;

extern int uploadNumberOfCharactor;
extern char uploadNameOfFile[64];

int SendCycle(int,char*,int);
int RecvCycle(int,char*,int);
int RecvCycBig(int,char*,int,int,int);
int RecvFile(int,char*);
int SendFile(int,char*);
int TcpInitClient(int*,char*,char*);

#define FILENAME "file"

#endif
