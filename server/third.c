#include "third.h"
#include "tranfile.h"
#define MEMORY_SEAT "../file/"
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
        MD5Update(&md5,data,ret);
        if(0 == ret||ret < 1024)
        {
            break;
        }
    }
    MD5Final(&md5,md5_value);
    for(i=0;i<16;i++)
    {
        snprintf(md5_str + i*2,2+1,"%02x",md5_value[i]);
    }
    md5_str[32]='\0';
    return 0;
}
int SendLs(int sockfd,MYSQL* conn)
{
    int size = sizeof("success");
    ShowDirectory(sockfd,conn);
    send(sockfd,&size,sizeof(int),0);
    send(sockfd,"success",size,0);
}
int SendPWD(int sockfd,char* pwd)
{
    train t;
    t.dataLen = strlen(pwd);
    strcpy(t.buf,pwd);
    SendCycle(sockfd,(char*)&t,4+t.dataLen);
    int size =sizeof("success");
    send(sockfd,&size,sizeof(int),0);
    send(sockfd,"success",size,0);
}
int SendCD(MYSQL* conn,char* name,char* pwd,char* ownername)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    char query[300]={0};
    int slash = strlen(name),i;
    char pwdbackup[256];
    char cashbuf[256]={0};
    int len = 0;
    strcpy(pwdbackup,pwd);
    for(i = 0;i<slash-1;i++)
    {
        name[i]=name[i+1];
        if('/'==name[i])
        {
            len++;
        }
    }
    name[i]='\0';
    if(!strcmp(name,".."))
    {
        len = 0;
        for(int i = strlen(pwd)-1;pwd[i]!='/';i--)
        {
            len++;
        }
        pwd[strlen(pwd)-len-1]='\0';
        strcpy(cashbuf,pwd);
        strcpy(pwd,cashbuf);
        slash = strlen(pwd);
        if(slash<5)
        {
            strcpy(pwd,"/home");
        }
        goto NextDir;
    }
    if(len)
    {
        goto NextDir;
    }
    strcpy(query,"select * from file where file_belong_directory='");
    sprintf(query,"%s%s' and file_owner='%s' and file_name='%s'",query,pwd,ownername,name);
    i=mysql_query(conn,query);
    if(i)
    {
        printf("Error making query:%s\n",mysql_error(conn));

    }else{
        res=mysql_store_result(conn);
        if(res)
        {
            i = 0;
            while((row=mysql_fetch_row(res))!=NULL)
            {
                i++;
                if(!strcmp(row[3],"d"))
                {
                    goto NextDir;
                }
            }
            if(2>i)
            {
                strcpy(pwd,pwdbackup);
                mysql_free_result(res);
                return -1;
            }
        }else{
            strcpy(pwd,pwdbackup);
            mysql_free_result(res);
            return -1;
        }
    }
    mysql_free_result(res);
NextDir:
    if(strcmp(name,".."))
    {
        slash = 0;
        for(int j = 0;j<i;j++)
        {
            if(name[j]=='/')
            {
                slash++;
            }
        }
        if(slash)
        {
            strcpy(pwd,name);
        }
        else
        {
            sprintf(pwd,"%s/%s",pwd,name);
        }
    }
    return 0;
}
int SendRM(MYSQL* conn,char* filename,char* pwd,char* ownername)
{
    char md5[32]={0};
    MYSQL_RES* res;
    MYSQL_ROW row;
    char fileRealName[64] = {0};
    char pwdbackup[256] = {0};
    char query[300]={0};
    int fnlen=strlen(filename);
    int i;
    for(i = 0;i<fnlen-1;i++)
    {
        filename[i]=filename[i+1];
    }
    filename[i]='\0';
    strcpy(query,"select * from file where file_belong_directory='");
    sprintf(query,"%s%s' and file_name='%s' and file_owner='%s'",query,pwd,filename,ownername);
    i=mysql_query(conn,query);
    if(i)
    {
        printf("Error making query:%s\n",mysql_error(conn));

    }else{
        res=mysql_store_result(conn);
        if(res)
        {
            i = 0;
            while((row=mysql_fetch_row(res))!=NULL)
            {
                i++;
                strcpy(md5,row[5]);
            }
            if(i==0)
            {
                return -1;
            }
        }else{
            return -1;
        }
    }
    //judge delete real file 
    memset(query,0,sizeof(query));
    strcpy(query,"select * from file where file_md5='");
    sprintf(query,"%s%s'",query,md5);
    i=mysql_query(conn,query);
    if(i)
    {
        printf("Error making query:%s\n",mysql_error(conn));

    }else{
        res=mysql_store_result(conn);
        if(res)
        {
            i = 0;
            while((row=mysql_fetch_row(res))!=NULL)
            {
                i++;
                strcpy(fileRealName,row[7]);
            }
            printf("i=%d\n",i);
            if(i==0)
            {
                return -1;
            }
            else if(1 == i)
            {
                sprintf(pwdbackup,"%s%s",MEMORY_SEAT,fileRealName);
                i = remove(pwdbackup);
                PUT_ERROR(i,-1,"remove");
            }
        }else{
            return -1;
        }
        memset(query,0,sizeof(query));
        strcpy(query,"delete from file where file_belong_directory='");
        sprintf(query,"%s%s' and file_name='%s' and file_owner='%s'",query,pwd,filename,ownername);
        i=mysql_query(conn,query);
        if(i)
        {
            printf("Error making query:%s\n",mysql_error(conn));
        }
    }
    return 0;
}
int RecvClient(int sockfd,MYSQL* conn,char* pwd,char* filename,char* ownername)
{
    char mfSize[33] = {0};
    char mfRName[64] = {0};
    char fnbuf[64] = {0};
    char MD5_str[33]={0};
    MYSQL_RES* res;
    MYSQL_ROW row;
    int fnlen=strlen(filename);
    int i,ret,mfRSize;
    char query[1024];
    for(i = 0;i<fnlen-1;i++)
    {
        filename[i]=filename[i+1];
    }
    filename[i]='\0';
    strcpy(fnbuf,filename);
CheckStart:
    memset(query,0,sizeof(query));
    strcpy(query,"select * from file where file_name='");
    sprintf(query,"%s%s' and file_owner='%s' and file_belong_directory='%s'",query,fnbuf,ownername,pwd);
    ret = mysql_query(conn,query);
    if(ret)
    {
        printf("记录到错误日志中去:%s\n",mysql_error(conn));
    }
    res = mysql_use_result(conn);
    if(res)
    {
        i=0;
        while((row = mysql_fetch_row(res))!=NULL)
        {
            i++;
        }
        if(i)
        {
            memset(fnbuf,0,sizeof(fnbuf));
            ret = send(sockfd,"error",5,0);
            ret = RecvCycle(sockfd,(char*)&mfRSize,sizeof(int));
            ret = RecvCycle(sockfd,fnbuf,mfRSize);
            goto CheckStart;
        }
        else
        {
            ret = send(sockfd,"okay!",5,0);
        }
    }
    i=0;
    ret = RecvCycle(sockfd,(char*)&i,sizeof(int));
    ret = RecvCycle(sockfd,mfSize,i);
    ret = RecvCycle(sockfd,MD5_str,32);
    PUT_ERROR(ret,-1,"RecvCycle(2/third.c)");
    memset(query,0,sizeof(query));
    strcpy(query,"select * from file where file_md5='");
    sprintf(query,"%s%s'",query,MD5_str);
    ret = mysql_query(conn,query);
    if(ret)
    {
        printf("记录到错误日志中去:%s\n",mysql_error(conn));
    }
    res = mysql_use_result(conn);
    if(res)
    {
        i=0;
        while((row=mysql_fetch_row(res))!=NULL)
        {
            i++;
            strcpy(mfSize,row[4]);
            strcpy(mfRName,row[7]);
        }
        if(!i)
        {
            memset(query,0,sizeof(query));
            strcpy(query,
                   "insert into file(file_name,file_belong_directory,file_type,file_size,file_md5,file_owner,file_real_name) values('");
            sprintf(query,"%s%s','%s','f','%s','%s','%s','%s')",query,fnbuf,pwd,mfSize,MD5_str,ownername,filename);
            ret = mysql_query(conn,query);
            if(ret)
            {
                printf("记录到错误日志中去:%s\n",mysql_error(conn));
            }
            send(sockfd,"giveme",6,0);
            //传递文件
            ret = RecvFile(sockfd);
            if(-1 == ret)
            {
                return -2;
            }
        }
        else
        {
            memset(query,0,sizeof(query));
            strcpy(query,
                   "insert into file(file_name,file_belong_directory,file_type,file_size,file_md5,file_owner,file_real_name) values('");
            sprintf(query,"%s%s','%s','f','%s','%s','%s','%s')",query,fnbuf,pwd,mfSize,MD5_str,ownername,mfRName);
            ret = mysql_query(conn,query);
            if(ret)
            {
                printf("记录到错误日志中去\n");
            }
            send(sockfd,"Ihave!",6,0);
        }
    }
    return 0;
}
int SendClient(int sockfd,MYSQL* conn,char* pwd,char* filename,char* ownername)
{
    char mfSize[33] = {0};
    char mfRName[64] = {0};
    char fnbuf[64] = {0};
    char MD5_str[33]={0};
    MYSQL_RES* res;
    MYSQL_ROW row;
    int fnlen=strlen(filename);
    int i,ret;
    char query[1024];
    for(i = 0;i<fnlen-1;i++)
    {
        filename[i]=filename[i+1];
    }
    filename[i]='\0';
    memset(query,0,sizeof(query));
    strcpy(query,"select * from file where file_name='");
    sprintf(query,"%s%s' and file_owner='%s' and file_belong_directory='%s'",query,filename,ownername,pwd);
    ret = mysql_query(conn,query);
    if(ret)
    {
        printf("记录到错误日志中去:%s\n",mysql_error(conn));
    }
    res = mysql_use_result(conn);
    if(res)
    {
        i=0;
        while((row = mysql_fetch_row(res))!=NULL)
        {
            i++;
            strcpy(mfRName,row[7]);
            strcpy(mfSize,row[4]);
            strcpy(MD5_str,row[5]);
        }
        if(!i)
        {
            strcpy(MD5_str,"12345123451234512345123451234512");
            ret = SendCycle(sockfd,MD5_str,32);
        }
        else
        {
            ret = SendCycle(sockfd,MD5_str,32);
SndFile:            
            ret = SendFile(sockfd,filename,mfRName);
            if(-1 == ret)
            {
                return -2;
            }
            ret = recv(sockfd,fnbuf,9,0);
            if(!strcmp(fnbuf,"lueluelue"))
            {
                goto SndFile;
            }
        }
    }
    return 0;
}
int ExecuteOparet(int sockfd,MYSQL* conn,char* pwd,char* ownername)
{
    train oc;
    memset(&oc,0,sizeof(oc));
    int cmdop;
    char path[256]={0};
    int ret=RecvCycle(sockfd,(char*)&oc.dataLen,sizeof(int));
    if(-1==oc.dataLen)
    {
        return -2;
    }
    ret = RecvCycle(sockfd,(char*)&oc.buf,oc.dataLen);
    cmdop = oc.buf[0]-'0';
    switch(cmdop)
    {
        //RECV_FLAG
    case 0:
        ret = RecvClient(sockfd,conn,pwd,oc.buf,ownername);
        if(-1==ret)
        {
            int size =strlen("Puts Failed!");
            send(sockfd,&size,sizeof(int),0);
            send(sockfd,"Puts Failed!",size,0);
        }
        if(0 == ret)
        {
            int size =sizeof("success!");
            send(sockfd,&size,sizeof(int),0);
            send(sockfd,"success!",size,0);
        }
        if(-2 == ret)
        {
            return -2;
        }
        break;
        //SEND_FLAG
    case 1:
        ret = SendClient(sockfd,conn,pwd,oc.buf,ownername);
        if(-1==ret)
        {
            int size =strlen("Gets Failed!");
            send(sockfd,&size,sizeof(int),0);
            send(sockfd,"Gets Failed!",size,0);
        }
        if(0 == ret)
        {
            int size =sizeof("success!");
            send(sockfd,&size,sizeof(int),0);
            send(sockfd,"success!",size,0);
        }
        if(-2 == ret)
        {
            return -2;
        }
        break;
        //LS_FLAG
    case 2:
        SendLs(sockfd,conn);
        break;
        //PWD_FLAG
    case 3:
        SendPWD(sockfd,pwd);
        break;
        //RM_FLAG
    case 4:
        ret = SendRM(conn,oc.buf,pwd,ownername);
        if(-1==ret)
        {
            int size =strlen("Not Found Directory!");
            send(sockfd,&size,sizeof(int),0);
            send(sockfd,"Not Found Directory!",size,0);
        }
        else
        {
            int size =sizeof("success");
            send(sockfd,&size,sizeof(int),0);
            send(sockfd,"success",size,0);
        }
        break;
        //CD_DIR_FLAG
    case 5:
        ret = SendCD(conn,oc.buf,pwd,ownername);
        if(-1 == ret)
        {
            int size =strlen("Not Found Directory!");
            send(sockfd,&size,sizeof(int),0);
            send(sockfd,"Not Found Directory!",size,0);
        }
        else
        {
            sprintf(path,"%s%s","pwd",pwd);
            int size =strlen(path);
            send(sockfd,&size,sizeof(int),0);
            send(sockfd,path,size,0);
        }
        break;
    }
}
