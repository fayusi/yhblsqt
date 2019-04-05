#include"factory.h"

int main(int argc,char **argv)
{
    ARGC_CHECK(argc,5);
    system("clear");
    int capacity = atoi(argv[4]);
    int threadNum = atoi(argv[3]);
    Factory_t factory;
    FactoryInit(&factory,threadNum,capacity);
    FactoryStart(&factory);
    int sockfd;
    TcpInitServer(&sockfd,argv[1],argv[2]);
    int new_fd;
    chdir("../file");
    pQue_t pq = &factory.que;
    while(1)
    {
        new_fd = accept(sockfd,NULL,NULL);
        pNode_t pnew = (pNode_t)calloc(1,sizeof(Node_t));
        pnew->new_fd = new_fd;
        pthread_mutex_lock(&pq->que_mutex);
        QueInsert(pq,pnew);
        pthread_mutex_unlock(&pq->que_mutex);
        pthread_cond_signal(&factory.cond);
    }
}
