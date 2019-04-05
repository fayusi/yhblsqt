#ifndef __HEAD_H__
#define __HEAD_H__
#include <mysql/mysql.h>
#include <crypt.h>
#include <signal.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/time.h>
#include <sys/mman.h>

#define ARGC_CHECK(argc,val) {if(argc!=val){printf("argc error!\n");return -1;}}
#define PUT_ERROR(val,num,name) {if(val==num){perror(name);return -1;}}
#define RETURN_MINUSONE(val,num,name) {if(val==num){return -1;}}
#define EXIT_ERROR(val,num,name) {if(val==num){perror(name);exit(-1);}}
#endif
