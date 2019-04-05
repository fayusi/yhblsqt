#ifndef __THIRD_H__
#define __THIRD_H__
#include "factory.h"
#include "md5.h"
typedef struct
{
    int size;
    char buf[512];
}optCommond_t,*pOptCommond_t;
int ExecuteOparet(int ,MYSQL* ,char*,char*);
#endif
