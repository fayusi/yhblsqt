#ifndef __MMYSQL_H__
#define __MMYSQL_H__
#include"head.h"
typedef struct
{
    char mfile_id[16];
    char mfile_name[64];
    char mfile_belong_directory[64];
    char mfile_type[2];
    char mfile_size[16];
    char mfile_md5[32];
}MyFile_t,*pMyFile_t;

#endif
