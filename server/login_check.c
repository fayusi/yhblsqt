#include"login_check.h"
#include"tranfile.h"
int CheckAccount(int sockfd,MYSQL* conn,char* usrname)
{
    char username[50]={0};
    char password[50]={0};
    char salt[10] = {0};
    int size;
    recv(sockfd,&size,sizeof(int),0);
    if(-1==size)
    {
        return -2;
    }
    recv(sockfd,username,size,0);
    strcpy(usrname,username);
    MYSQL_RES* res;
    MYSQL_ROW row;
    char query[300] = "select * from user where username='";
    sprintf(query,"%s%s%s",query,username,"'");
    int ret=mysql_query(conn,query);
    if(ret)
    {
        printf("Error making query:%s\n",mysql_error(conn));
    }
    res = mysql_store_result(conn);
    unsigned int i = mysql_num_rows(res);
    if(!i)
    {
        strcpy(salt,"dd");
        send(sockfd,salt,sizeof(salt),0);
        recv(sockfd,password,sizeof(password),0);
        send(sockfd,"username error!",sizeof(char)*20,0);
        return -1;
    }
    row = mysql_fetch_row(res);
    strcpy(salt,row[2]);
    send(sockfd,row[2],sizeof(salt),0);
    recv(sockfd,password,sizeof(password),0);
    if(res)
    {
        if(!strcmp(password,row[1]))
        {
            send(sockfd,"login success!",sizeof(char)*20,0);
        }
        else
        {
            send(sockfd,"password error!",sizeof(char)*20,0);
            return -1;
        }
    }
    else
    {
        send(sockfd,"Don't find data",sizeof(char)*20,0);
    }
    mysql_free_result(res);
    return 0;
}
int LinkMysql(MYSQL** conn)
{
    char server[]="localhost";
    char user[] = "root";
    char password[] = "xjl24568";
    char database[] = "netdisk";
    *conn = mysql_init(NULL);
    if(!mysql_real_connect(*conn,server,user,password,database,0,NULL,0))
    {
        printf("Error connecting to database:%s\n",mysql_error(*conn));
        return -1;
    }
    return 0;
}
char* getsalt()                                     
{                                                   
    char str[11] = {0};                             
    int i,flag;                                     
    srand(time(NULL));                              
    for(i = 0;i < 10;i++)                           
    {                                               
        flag = rand()%3;                            
        switch(flag)                                
        {                                           
        case 0:                                     
            str[i] = rand()%26+'a';                 
            break;                                  
        case 1:                                     
            str[i] = rand()%26+'A';                 
            break;                                  
        case 2:                                     
            str[i] = rand()%10+'0';                 
            break;                                  
        }                                           
    }                                               
    char* ret;                                      
    char* salt = (char*)calloc(sizeof(char),10);    
    char key[11] = "Br13J";                         
    ret = crypt(key,str);                           
    for(i = 0;i<10;i++)                             
    {                                               
        salt[i] = ret[i];                           
    }                                               
    return salt;                                    
}                                                   
int RegistAccount(int sockfd,MYSQL* conn)
{
    //recv username and password
    char password[32] = {0};
    char username[32] = {0};
    int size;
    recv(sockfd,&size,sizeof(int),0);
    if(-1==size)
    {
        return -2;
    }
    recv(sockfd,username,size,0);
    recv(sockfd,password,sizeof(password),0);
    //register account
    char pw[128] = {0};
    char* s=getsalt();
    char* r = crypt(password,s);
    strcpy(pw,r);
    char operatTable[512] = "insert into user(username,password,salt) values('";
    sprintf(operatTable,"%s%s','%s','%s')",operatTable,username,pw,s);
    int ret = mysql_query(conn,operatTable);
    if(ret)
    {
        send(sockfd,"The user name is already in use!",sizeof(char)*35,0);
        return -1;
    }
    send(sockfd,"registered successfully!",sizeof(char)*30,0);
    return 0;
}
int ShowDirectory(int sockfd,MYSQL* conn)
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    train t;
    memset(&t,0,sizeof(train));
    pMyFile_t file = (pMyFile_t)calloc(1,sizeof(MyFile_t));
    char belongdir[64];
    char ownername[64];
    RecvCycle(sockfd,(char*)&t.dataLen,sizeof(int));
    RecvCycle(sockfd,(char*)&t.buf,t.dataLen);
    strcpy(belongdir,t.buf);
    memset(&t,0,sizeof(train));
    RecvCycle(sockfd,(char*)&t.dataLen,sizeof(int));
    RecvCycle(sockfd,(char*)&t.buf,t.dataLen);
    strcpy(ownername,t.buf);
    char query[300] = "select * from file where file_belong_directory='";
    sprintf(query,"%s%s' and file_owner='%s' order by file_name",query,belongdir,ownername);
    int ret = mysql_query(conn,query);
    if(ret)
    {
        send(sockfd,"The user name is already in use!",sizeof(char)*35,0);
        return -1;
    }
    else
    {
        res = mysql_store_result(conn);
        if(res)
        {
            while((row=mysql_fetch_row(res))!=NULL)
            {
                strcpy(file->mfile_name,row[1]);
                strcpy(file->mfile_type,row[3]);
                strcpy(file->mfile_size,row[4]);
                strcpy(file->mfile_md5,row[5]);
                strcpy(file->mfile_id,row[0]);
                sprintf(t.buf,"%s$%s$%s$%s$%s$",file->mfile_type,file->mfile_name,file->mfile_size,file->mfile_md5,file->mfile_id);
                t.dataLen = strlen(t.buf);
                SendCycle(sockfd,(char*)&t,4+t.dataLen);
            }
            t.dataLen = 0;
            SendCycle(sockfd,(char*)&t,4);
        }
    }
    free(file);
    file=NULL;
}
