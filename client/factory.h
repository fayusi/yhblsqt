#ifndef __FACTORY_H__
#define __FACTORY_H__
#include"tranfile.h"
#include"head.h"
#include"mmysql.h"
#define SEND_FLAG {(int)1}
#define RECV_FLAG {(int)0}
#define LS_FLAG {(int)2}
#define PWD_FLAG {(int)3}
#define CD_PATH_FLAG {(int)4}
#define CD_DIR_FLAG {(int)5}
#define RM_FLAG {(int)6}
#define EXIT_FLAG 99

typedef struct
{
    int size;
    char buf[512];
}optCommond_t,*pOptCommond_t;

int SendOparet(int,char*,char*);
int LSCommond(char* ,char* ,int);

#endif
