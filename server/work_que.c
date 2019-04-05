#include"work_que.h"
void QueInit(pQue_t pq,int capacity)
{
    memset(pq,0,sizeof(Que_t));
    pq->que_capacity = capacity;
    pthread_mutex_init(&pq->que_mutex,NULL);
}
void QueInsert(pQue_t pq,pNode_t pnode)
{
    if(0 == pq->que_size)
    {
        pq->que_front = pnode;
        pq->que_rear = pnode;
    }
    else
    {
        pq->que_rear->pNext = pnode;
        pq->que_rear = pnode;
    }
    pq->que_size++;
}
int QueGet(pQue_t pq,pNode_t* ppnode)
{
    if(0 == pq->que_size)
    {
        return -1;
    }
    else
    {
        *ppnode = pq->que_front;
        pq->que_front = pq->que_front->pNext;
        if(NULL == pq->que_front)
        {
            pq->que_rear = NULL;
        }
        pq->que_size--;
    }
    return 0;
}

