#ifndef __FACTORY_H__
#define __FACTORY_H__
#include "head.h"
#include "work_que.h"
#include "login_check.h"

extern int uploadNumberOfCharactor;
extern char uploadNameOfFile[64];

typedef struct
{
    pthread_t* pthid;
    int threadNum;
    pthread_cond_t cond;
    short startFlag;
    short endFlag;
    Que_t que;
}Factory_t,*pFactory_t;
void FactoryInit(pFactory_t,int,int);
void FactoryStart(pFactory_t);
int TcpInitServer(int*,char*,char*);

#endif
