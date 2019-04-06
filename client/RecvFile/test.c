#include<func.h>

int main(int argc,char** argv)
{
    ARGC_CHECK(argc,2);
    int fd = open(argv[1],O_RDWR);
    struct stat ss;
    fstat(fd,&ss);
    int bn = ss.st_blocks;
    int total = bn*512;
    printf("bn=%d,total=%d\n",bn,total);
    close(fd);
}
