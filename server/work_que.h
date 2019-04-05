#ifndef __WORK_QUE_H__
#define __WORK_QUE_H__
#include"head.h"
typedef struct tag
{
    MYSQL* conn;
    int new_fd;
    struct tag* pNext;
}Node_t,*pNode_t;
typedef struct
{
    pNode_t que_front;
    pNode_t que_rear;
    int que_capacity;
    int que_size;
    pthread_mutex_t que_mutex;
}Que_t,*pQue_t;

void QueInit(pQue_t,int);
void QueInsert(pQue_t,pNode_t);
int QueGet(pQue_t,pNode_t*);

#endif
