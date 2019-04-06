#include"factory.h"
#include"mmysql.h"
int sockfd;
int uploadNumberOfCharactor;
char uploadNameOfFile[64]={0};
void sigfunc(int num)
{
    int cmd = -1;
    send(sockfd,&cmd,sizeof(int),0);
    close(sockfd);
    printf("logout!\n");
    if(uploadNumberOfCharactor)
    {
        int fd = open(uploadNameOfFile,O_WRONLY);
        sprintf(uploadNameOfFile,"%d",uploadNumberOfCharactor);
        write(fd,uploadNameOfFile,strlen(uploadNameOfFile));
        close(fd);
    }
    exit(1);
}
int LoginAccount(int sockfd,char* un)
{
    char username[50] = {0};
    char* password;
    char salt[10] = {0};
    char error[7] = {0};
    int size;
    printf("Please enter your username:\n");
    read(STDIN_FILENO,username,sizeof(username));
    username[strlen(username)-1] = '\0';
    int usernamelen = strlen(username);
    if(usernamelen>30)
    {
        printf("The user name is too long\n");
        return -1;
    }
    for(int i = 0;i<usernamelen-1;i++)
    {
        if((username[i]>='a'&&username[i]<='z')||(username[i]>='A'&&username[i]<='Z')||
           username[i]=='_'||(username[i]>='0'&&username[i]<='9'))
        {
            continue;   
        }
        else
        {
            printf("Error Charactor!\n");
            return -1;
        }

    }
    strcpy(un,username);
    size = strlen(username);
    send(sockfd,&size,sizeof(int),0);
    send(sockfd,username,size,0);
    recv(sockfd,salt,sizeof(salt),0);
    password = getpass("Please enter your password:");
    password = crypt(password,salt);
    send(sockfd,password,strlen(password),0);
    memset(username,0,sizeof(username));
    recv(sockfd,username,sizeof(username),0);
    printf("%s\n",username);
    for(int i = strlen(username)-1,m = 0;m<6;m++,i--)
    {
        error[5-m] = username[i];
    }
    error[6]='\0';
    if(!strcmp(error,"error!"))
    {
        return -1;
    }
    memset(username,0,sizeof(username));
    return 0;
}
int RegistAccount(int sockfd)
{
    char username[32] = {0};
    char* password;
    char message[64] = {0};
    char error[4] = {0};
    printf("Please enter your username:\n");
    read(STDIN_FILENO,username,sizeof(username));
    int usernamelen = strlen(username);
    username[usernamelen-1] = '\0';
    if(usernamelen>30)
    {
        printf("The user name is too long\n");
        return -1;
    }
    for(int i = 0;i<usernamelen-1;i++)
    {
        if((username[i]>='a'&&username[i]<='z')||(username[i]>='A'&&username[i]<='Z')||
           username[i]=='/'||username[i]=='_'||username[i]=='.'||username[i]=='+'||
           username[i]=='-'||username[i]=='*'||(username[i]>='0'&&username[i]<='9'))
        {
            continue;   
        }
        else
        {
            printf("Error Charactor!\n");
            return -1;
        }

    }
    send(sockfd,&usernamelen,sizeof(int),0);
    send(sockfd,username,usernamelen,0);
    password = getpass("Please enter your password:");
    printf("%ld\n",strlen(password));
    if(strlen(password)>30)
    {
        printf("The password is too long\n");
        return -1;
    }
    send(sockfd,password,strlen(password),0);
    recv(sockfd,message,sizeof(message),0);
    for(int i = strlen(username)-1,m = 0;m<4;m++,i--)
    {
        error[2-m] = username[i];
    }
    error[3]='\0';
    if(!strcmp(error,"se!"))
    {
        return -1;
    }
    puts(message);
    return 0;
}
int main(int argc,char** argv)
{
    char pwd[256]="/home";
    train t;
    memset(&t,0,sizeof(train));
    ARGC_CHECK(argc,3);
    chdir("RecvFile");
    int optcmd,ret;
    char userSelect[8] = {0};
    char username[50] = {0};
    sockfd = 0;
    TcpInitClient(&sockfd,argv[1],argv[2]);
    system("clear");
    signal(SIGINT,sigfunc);
    while(1)
    {
LogStart:
        system("clear");
        memset(userSelect,0,sizeof(userSelect));
        printf("1.Login Account\n2.Registered Account\nSelect num:\n");
        read(STDIN_FILENO,userSelect,sizeof(userSelect));
        if(strlen(userSelect)>2)
        {
            printf("Commond Error!\n");
            goto LogStart;
        }
        if(userSelect[0]>'9'||userSelect[0]<'0')
        {
            printf("Commond Error\n");
            goto LogStart;
        }
        userSelect[strlen(userSelect)-1] = '\0';
        optcmd = 19+atoi(userSelect);
        send(sockfd,&optcmd,sizeof(int),0);
        switch(optcmd)
        {
        case 20:
LogAcc:            
            ret = LoginAccount(sockfd,username);
            if(ret == -1)
            {
                goto LogAcc;
            }
            if(!ret)
            {
                goto ClientStart;
            }
            break;
        case 21:
            ret = RegistAccount(sockfd);
            if(-1 == ret)
            {
                goto LogStart;
            }
            system("clear");
            break;
        default:
            printf("Commond Error!\n");
            goto LogStart;
            break;
        }
    }
ClientStart:    
    LSCommond(username,pwd,sockfd);
    while(1)
    {
        SendOparet(sockfd,username,pwd);
    }
}
