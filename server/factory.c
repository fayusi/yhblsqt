#include"factory.h"
#include"third.h"
int uploadNumberOfCharactor;
char uploadNameOfFile[64] = {0};
void cleanFunc(void* pArg)
{
    pFactory_t pf = (pFactory_t)pArg;
    printf("unlock\n");
    pthread_mutex_unlock(&pf->que.que_mutex);
    if(uploadNumberOfCharactor)
    {
        int fd = open(uploadNameOfFile,O_WRONLY);
        sprintf(uploadNameOfFile,"%d",uploadNumberOfCharactor);
        write(fd,uploadNameOfFile,strlen(uploadNameOfFile));
        close(fd);
    }
}
void* creatThread(void* factory)
{
    pFactory_t pf = (pFactory_t)factory;
    pNode_t pcur;
    int getSuccess,ret;
    static int clientnum;
    MYSQL* conn;
    char username[64]={0};
    //recv commond about register login or other
    int optcmd;
    while(1)
    {
Rebirth:        
        getSuccess = 1;
        pthread_cleanup_push(cleanFunc,(void*)pf);
        pthread_mutex_lock(&pf->que.que_mutex);
        printf("endFlag=%d\n",pf->endFlag);
        if(pf->endFlag)
        {
            clientnum=0;
            pthread_exit(NULL);
        }
        if(!pf->que.que_size)
        {
            puts("thread wait");
            pthread_cond_wait(&pf->cond,&pf->que.que_mutex);
            puts("thread start");
        }
        getSuccess = QueGet(&pf->que,&pcur);    
        clientnum++;
        printf("%d client connected\n",clientnum);
        pthread_mutex_unlock(&pf->que.que_mutex);
        pthread_cleanup_pop(0);
        if(!getSuccess)
        {
            LinkMysql(&conn);
            char pwd[256] = "/home";
LogStart:
            recv(pcur->new_fd,&optcmd,sizeof(int),0);
            if(-1 == optcmd)
            {
                printf("client disconnect\n");
                close(pcur->new_fd);
                clientnum--;
                goto Rebirth;
            }
            switch(optcmd)
            {
            case 20:
LogAcc:        
                ret = CheckAccount(pcur->new_fd,conn,username);
                if(-1 == ret)
                {
                    goto LogAcc;
                }
                else if(-2 == ret)
                {
                    printf("client disconnect\n");
                    close(pcur->new_fd);
                    clientnum--;
                    goto Rebirth;
                }
                break;
            case 21:
                ret = RegistAccount(pcur->new_fd,conn);
                if(-1 == ret)
                {
                    goto LogStart;
                }
                else if(-2 == ret)
                {
                    printf("client disconnect\n");
                    close(pcur->new_fd);
                    clientnum--;
                    goto Rebirth;
                }
                else if(0 == ret)
                {
                    goto LogStart;
                }
                break;
            }
            ShowDirectory(pcur->new_fd,conn);
            while(1)
            {
                ret = ExecuteOparet(pcur->new_fd,conn,pwd,username); 
                if(-2==ret)
                {
                    printf("client disconnect\n");
                    close(pcur->new_fd);
                    clientnum--;
                    goto Rebirth;
                }
            }
        }
    }

}
void FactoryInit(pFactory_t factory,int threadNum,int capacity)
{
    memset(factory,0,sizeof(Factory_t));
    factory->threadNum = threadNum;
    factory->pthid = (pthread_t*) calloc(threadNum,sizeof(pthread_t));
    pthread_cond_init(&factory->cond,NULL);
    QueInit(&factory->que,capacity);
}
void FactoryStart(pFactory_t factory)
{
    int creatNum;
    if(0 == factory->startFlag)
    {
        for(creatNum = 0;creatNum < factory->threadNum;creatNum++)
        {
            pthread_create(factory->pthid,NULL,creatThread,factory);
        }
    }
    factory->startFlag = 1;
}
