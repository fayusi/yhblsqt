#include"head.h"

int TcpInitServer(int *sockfd,char *ip,char* port)
{
    int sfd = socket(AF_INET,SOCK_STREAM,0);
    PUT_ERROR(sfd,-1,"socket");
    struct sockaddr_in client;
    memset(&client,0,sizeof(client));
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(ip);
    client.sin_port = htons(atoi(port));
    int reuse = 1;
    int ret = setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int));
    PUT_ERROR(-1,ret,"setsockopt");
    ret = bind(sfd,(struct sockaddr*)&client,sizeof(struct sockaddr));
    PUT_ERROR(-1,ret,"bind");
    ret = listen(sfd,10);
    PUT_ERROR(-1,ret,"listen");
    *sockfd = sfd;
    return 0;
}

int TcpInitClient(int *sfd,char *ip,char* port)
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    PUT_ERROR(sockfd,-1,"socket");
    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(ip);
    client.sin_port = htons(atoi(port));
    int ret = connect(sockfd,(struct sockaddr*)&client,sizeof(struct sockaddr));
    PUT_ERROR(ret,-1,"connect");
    *sfd = sockfd;
    return 0;
}
