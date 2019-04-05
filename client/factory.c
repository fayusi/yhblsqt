#include"factory.h"
#include"md5.h"
int Compute_file_md5(const char *file_path,char *md5_str)
{
    int i,fd,ret;
    unsigned char data[1024]={0};
    unsigned char md5_value[16];
    MD5_CTX md5;

    fd = open(file_path,O_RDONLY);
    PUT_ERROR(fd,-1,"open");
    MD5Init(&md5);
    while(1)
    {
        ret = read(fd,data,1024);
        if(-1 == ret)
        {
            perror("read");
            close(fd);
            return -1;
        }
        MD5Update(&md5,data,ret);
        if(0 == ret || ret < 1024)
        {
            break;
        }
    }
    close(fd);
    MD5Final(&md5,md5_value);
    for(i = 0;i<16;i++)
    {
        snprintf(md5_str + i*2,2+1,"%02x",md5_value[i]);
    }
    return 0;
}
int Block_check_md5(int fd,int splitsize,char* md5_str)
{
    unsigned char data[1024] = {0};
    unsigned char md5_value[16];
    int total = 0;
    int ret = 0;
    int i;
    MD5_CTX md5;
    while (splitsize>total)
    {
        ret = read(fd, data,1024);
        if (-1 == ret)
        {
            perror("read");
            return -1;
        }
        total +=ret;
        MD5Update(&md5, data, ret);
        if (0 == ret || ret < 1024)
        {
            break;
        }
    }
    MD5Final(&md5, md5_value);
    for(i = 0; i < 16; i++)
    {
        snprintf(md5_str + i*2, 2+1, "%02x", md5_value[i]);
    }
    md5_str[32] = '\0'; // add end
    return 0;
}
int SendGets(int sockfd,char* filename)
{
    train t;
    memset(&t,0,sizeof(train));
    t.buf[0] = '1';
    sprintf(t.buf,"%s%s",t.buf,filename);
    t.dataLen = strlen(t.buf);
    int ret = SendCycle(sockfd,(char*)&t,4+t.dataLen);
    PUT_ERROR(-1,ret,"send(SendGets)");
    //recv md5sum
    char md5[33]={0};
    ret = RecvCycle(sockfd,md5,32);
    if(!strcmp(md5,"12345123451234512345123451234512"))
    {
        printf("Not Found File!\n");
        return -1;
    }
RcvFile:
    RecvFile(sockfd);

    char md5_str[33]={0};
    ret = Compute_file_md5(filename,md5_str);
    if(ret)
    {
        printf("md5 builder crash!\n");
        return -1;
    }
    if(!strcmp(md5,md5_str))
    {
        ret = send(sockfd,"success!!",9,0);
        printf("get complete file!\n");
    }
    else
    {
        ret = send(sockfd,"lueluelue",9,0);
        goto RcvFile;
    }
    return 0;
}
int SendPuts(int sockfd,char* filename)
{
    char RFname[64] = {0};
    int fncnt = 0,i;
    char md5_str[33]={0};
    char sendSig[7]={0};
    int fnlen = strlen(filename);
    int fd = open(filename,O_RDONLY);
    if(-1 == fd)
    {
        perror("open");
        return -1;
    }
    struct stat ss;
    fstat(fd,&ss);
    int size = ss.st_size;
    train t;
    memset(&t,0,sizeof(train));
    t.buf[0] = '0';
    t.dataLen = Compute_file_md5(filename,md5_str);
    if(t.dataLen)
    {
        return -1;
    }
    for(i=fnlen-1;filename[i]!='/'&&i>=0;i--);
    if(i) i++;
    for(;i<fnlen;i++)
    {
        filename[fncnt] = filename[i];
        fncnt++;
    }
    filename[fncnt] = '\0';
    strcpy(RFname,filename);
SendFileName:
    sprintf(t.buf,"%s%s",t.buf,RFname);
    t.dataLen = strlen(t.buf);
    int ret = SendCycle(sockfd,(char*)&t,4+t.dataLen);
    PUT_ERROR(-1,ret,"send(SendGets)");
    recv(sockfd,sendSig,5,0);
    if(!strcmp(sendSig,"error"))
    {
        printf("Enter filename not exist in current dirctory:\n");
ReEnter:        
        ret = read(STDIN_FILENO,RFname,sizeof(RFname));
        RFname[ret - 1] = '\0';
        if(ret>64)
        {
            printf("filename too long to the file\n");
            goto ReEnter;
        }
        for(int i = 0;i<ret-1;i++)
        {
            if((RFname[i]>='a'&&RFname[i]<='z')||(RFname[i]>='A'&&RFname[i]<='Z')||
               RFname[i]=='/'||RFname[i]=='_'||RFname[i]=='.'||RFname[i]=='-'||
               (RFname[i]>='0'&&RFname[i]<='9'))
            {
                continue;
            }
            else
            {
                printf("Enter illegal character!\n");
                goto ReEnter;
            }
        }
        memset(t.buf,0,sizeof(t.buf));
        goto SendFileName;
    }
    memset(t.buf,0,sizeof(t.buf));
    sprintf(t.buf,"%d",size);
    t.dataLen = strlen(t.buf);
    ret = SendCycle(sockfd,(char*)&t,4+t.dataLen);
    PUT_ERROR(-1,ret,"send(SendGets)");
    close(fd);
    memset(t.buf,0,sizeof(t.buf));
    strcpy(t.buf,md5_str);
    ret = SendCycle(sockfd,t.buf,32);
    PUT_ERROR(-1,ret,"send(SendGets)");
    recv(sockfd,sendSig,6,0);
    if(!strcmp(sendSig,"giveme"))
    {
        SendFile(sockfd,filename);
    }
    return 0;
}

int LSCommond(char* username,char* dirctory,int sockfd)
{
    system("clear");
    printf("%-5s%32s%32s\n","type","file_name","file_size");
    MyFile_t FileInfo;
    memset(&FileInfo,0,sizeof(FileInfo));
    train t;
    memset(&t,0,sizeof(train));
    int j = 0;
    int bufsize;
    char split[5] = {0};
    t.dataLen = strlen(dirctory);
    strcpy(t.buf,dirctory);
    SendCycle(sockfd,(char*)&t,4+t.dataLen);
    memset(&t,0,sizeof(train));
    t.dataLen = strlen(username);
    strcpy(t.buf,username);
    SendCycle(sockfd,(char*)&t,4+t.dataLen);
    while(1)
    {
        RecvCycle(sockfd,(char*)&t.dataLen,sizeof(int));
        if(0 == t.dataLen)
        {
            break;
        }
        RecvCycle(sockfd,t.buf,t.dataLen);
        j = 0;
        for(unsigned long i=0;i<strlen(t.buf);i++)
        {

            if('$' == t.buf[i])
            {
                split[j] = i;
                j++;
            }
        }
        bufsize = 0;
        for(j = 0;j<split[0];j++)
        {
            FileInfo.mfile_type[j] = t.buf[bufsize];
            bufsize++;
        }
        bufsize++;
        for(j = 0;j<split[1]-split[0]-1;j++)
        {
            FileInfo.mfile_name[j] = t.buf[bufsize];
            bufsize++;
        }
        bufsize++;
        for(j = 0;j<split[2]-split[1]-1;j++)
        {
            FileInfo.mfile_size[j] = t.buf[bufsize];
            bufsize++;
        }
        bufsize++;
        for(j = 0;j<split[3]-split[2]-1;j++)
        {
            FileInfo.mfile_md5[j] = t.buf[bufsize];
            bufsize++;
        }
        bufsize++;
        for(j = 0;j<split[4]-split[3]-1;j++)
        {
            FileInfo.mfile_id[j] = t.buf[bufsize];
            bufsize++;
        }
        printf("%-4s%32s%32s\n",FileInfo.mfile_type,FileInfo.mfile_name,FileInfo.mfile_size);
        memset(&t,0,sizeof(train));
        memset(&FileInfo,0,sizeof(MyFile_t));
    }
    return 0;
}
int SendLs(int sockfd)
{
    train oc;
    memset(&oc,0,sizeof(train));
    system("clear");
    oc.buf[0] = '2';
    oc.dataLen = strlen(oc.buf);
    int ret = SendCycle(sockfd,(char*)&oc,4+oc.dataLen);
    PUT_ERROR(-1,ret,"send(SendLS)");
    return 0;
}
int SendPWD(int sockfd,char* pwd)
{
    train oc;
    memset(&oc,0,sizeof(train));
    oc.buf[0] = '3';
    oc.dataLen = strlen(oc.buf);
    int ret = SendCycle(sockfd,(char*)&oc,4+oc.dataLen);
    PUT_ERROR(-1,ret,"send(SendPWD)");
    int size = 0;
    char infobuf[256] = {0};
    RecvCycle(sockfd,(char*)&size,sizeof(int));
    RecvCycle(sockfd,infobuf,size);
    printf("%s\n",infobuf);
    strcpy(pwd,infobuf);
    return 0;
}
int SendRM(int sockfd,char* name)
{
    optCommond_t oc;
    memset(&oc,0,sizeof(oc));
    oc.buf[0] = '4';
    sprintf(oc.buf,"%s%s",oc.buf,name);
    oc.size = strlen(oc.buf);
    int ret = SendCycle(sockfd,(char*)&oc,4+oc.size);
    PUT_ERROR(-1,ret,"send(SendRM)");
    return 0;
}
int EnterDir(int sockfd,char* name)
{
    train oc;
    memset(&oc,0,sizeof(train));
    oc.buf[0] = '5';
    sprintf(oc.buf,"%s%s",oc.buf,name);
    oc.dataLen = strlen(oc.buf);
    int ret = SendCycle(sockfd,(char*)&oc,4+oc.dataLen);
    PUT_ERROR(-1,ret,"send(SendRM)");
    ret = strlen(oc.buf);
    return 0;
}

int SendOparet(int sockfd,char* owner,char* pwd)
{
    int retval = 0;
    char pwdbackup[256] = {0};
    int size = 0,cnt;
    char oprbuf[256] = {0};
    char cmdbuf[4] = {0};
    char srbuf[6]={0};
Start:
    puts("enter:");
    memset(oprbuf,0,sizeof(oprbuf));
    int ret = read(STDIN_FILENO,oprbuf,sizeof(oprbuf));
    oprbuf[strlen(oprbuf)-1] = '\0';
    PUT_ERROR(-1,ret,"send(SendGets)");
    cnt = 0;
    for(unsigned long i = 0;i<strlen(oprbuf);i++)
    {
        if(ret<3)
        {
            printf("Commond Error!\n");
            goto Start;
        }
        if(oprbuf[i]==' ')
        {
            cnt=i;
        }
    }
    if(cnt>4)
    {
        printf("Commond Error!\n");
        goto Start;
    }
    for(int i=0;i<5;i++)
        srbuf[i] = oprbuf[i];
    srbuf[strlen(srbuf)] = '\0';
    for(int i=0;i<3;i++)
        cmdbuf[i] = oprbuf[i];
    cmdbuf[strlen(cmdbuf)] = '\0';
    size = 0;
    for(unsigned long i = cnt+1;i<strlen(oprbuf);i++)
    {
        if((oprbuf[i]>='a'&&oprbuf[i]<='z')||(oprbuf[i]>='A'&&oprbuf[i]<='Z')||
           oprbuf[i]=='/'||oprbuf[i]=='_'||oprbuf[i]=='.'||oprbuf[i]=='-'||
           (oprbuf[i]>='0'&&oprbuf[i]<='9'))
        {
            oprbuf[size] = oprbuf[i];
            size++;
        }
        else
        {
            printf("Enter filename or routine error!\n");
            goto Start;
        }
    }
    oprbuf[size] = '\0';
    if(ret<5)
    {
        if(!strcmp(srbuf,"pwd"))
        {
            SendPWD(sockfd,pwd);
        }
        else if(!strcmp(srbuf,"ls"))
        {
            SendLs(sockfd);
            LSCommond(owner,pwd,sockfd);
        }
        else if(!strcmp(srbuf,"rm "))
        {
            SendRM(sockfd,oprbuf);
        }
        else if(!strcmp(srbuf,"cd "))
        {
            strcpy(pwdbackup,pwd);
            EnterDir(sockfd,oprbuf);    
        }
        else
        {
            printf("Commond Error!\n");
            goto operatend;
        }
    }
    else
    {
        if(!strcmp(cmdbuf,"cd "))
        {
            strcpy(pwdbackup,pwd);
            EnterDir(sockfd,oprbuf);
        }
        else if(!strcmp(cmdbuf,"rm "))
        {
            SendRM(sockfd,oprbuf);
        }
        else if(!strcmp(srbuf,"gets "))
        {
            puts("gets");
            SendGets(sockfd,oprbuf);
        }
        else if(!strcmp(srbuf,"puts "))
        {
            retval = SendPuts(sockfd,oprbuf);
            if(-1==retval)
            {
                goto operatend;
            }
        }
        else
        {
            printf("Commond Error!\n");
            goto operatend;
        }
    }
    ret = recv(sockfd,&size,sizeof(int),0);
    ret = recv(sockfd,oprbuf,size,0);
    if(!strcmp(oprbuf,"Not Found Directory!"))
    {
        puts(oprbuf);
        strcpy(pwd,pwdbackup);
    }
    else if(!strcmp(oprbuf,"success!"))
    {
        puts(oprbuf);
    }
    else
    {
        int i;
        for(i = 0;i<3;i++)
        {
            srbuf[i] = oprbuf[i];
        }
        srbuf[3] = '\0';
        if(!strcmp(srbuf,"pwd"))
        {
            memset(pwdbackup,0,sizeof(char)*256);
            i = strlen(oprbuf);
            for( ret=3;ret<i;ret++)
            {
                pwdbackup[ret-3]=oprbuf[ret];
            }
            strcpy(pwd,pwdbackup);
        }
    }
    PUT_ERROR(-1,ret,"send(SendGets)");
operatend:    
    return 0;
}

