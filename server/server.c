#include"factory.h"
int exitFlags[2];
void sigFunc(int arg)
{
    puts("mogui");
    write(exitFlags[1],&arg,4);
}
int main(int argc,char **argv)
{
    ARGC_CHECK(argc,5);
    system("clear");
    int ret;
    int capacity = atoi(argv[4]);
    int threadNum = atoi(argv[3]);
    Factory_t factory;
    int sockfd;
    int new_fd;
    pQue_t pq = &factory.que;
    pid_t pid;
    chdir("../file");
    FactoryInit(&factory,threadNum,capacity);
    FactoryStart(&factory);
    TcpInitServer(&sockfd,argv[1],argv[2]);

    signal(SIGINT,sigFunc);    
    pipe(exitFlags);

    int epfd = epoll_create(1);
    struct epoll_event event,evs[2];
    event.events = EPOLLIN;
    event.data.fd = sockfd;
    ret = epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&event);
    PUT_ERROR(ret,-1,"epoll_ctl0");
    event.events = EPOLLIN;
    event.data.fd = exitFlags[0];
    ret = epoll_ctl(epfd,EPOLL_CTL_ADD,exitFlags[0],&event);
    PUT_ERROR(ret,-1,"epoll_ctl0");
    int readyNum;
    if(0 == pid)
    {
        while(1)
        {
            readyNum = epoll_wait(epfd,evs,2,-1);
            printf("%d\n",readyNum);
            if(-1 != readyNum)
            {
                for(int i=0;i<readyNum;i++)
                {
                    if(evs[i].data.fd == sockfd)
                    {
                            new_fd = accept(sockfd,NULL,NULL);
                            pNode_t pnew = (pNode_t)calloc(1,sizeof(Node_t));
                            pnew->new_fd = new_fd;
                            pthread_mutex_lock(&pq->que_mutex);
                            QueInsert(pq,pnew);
                            pthread_mutex_unlock(&pq->que_mutex);
                            pthread_cond_signal(&factory.cond);
                    }
                    if(exitFlags[0] == evs[i].data.fd)
                    {
                        read(exitFlags[0],&ret,4);
                        factory.endFlag=1;
                        for(i = 0;i<threadNum;i++)
                        {
                            printf("pthread %d ready exit!\n",i);
                            pthread_cond_broadcast(&factory.cond);
                            pthread_join(factory.pthid[i],NULL);
                            printf("pthread %d exit!\n",i);
                        }
                        exit(1);
                    }
                }   
            }
        }
    }
    wait(NULL);
}
