#ifndef __LOGIN_CHECK_H__
#define __LOGIN_CHECK_H__
#include"head.h"
typedef struct
{
    char mfile_owner[64];
    char mfile_real_name[64];
    char mfile_id[16];
    char mfile_name[64];
    char mfile_belong_directory[256];
    char mfile_type[2];
    char mfile_size[16];
    char mfile_md5[32];
}MyFile_t,*pMyFile_t;
int CheckAccount(int,MYSQL*,char*);
int LinkMysql(MYSQL**);
int RegistAccount(int,MYSQL*);
int ShowDirectory(int,MYSQL*);

#endif
