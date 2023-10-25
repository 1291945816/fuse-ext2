#define FUSE_USE_VERSION      26

#include<stdio.h>
#include<time.h>
#include <fuse/fuse.h>

#include"fuse-ext2/types.h"
#include "device.h"


static struct fuse_operations fext2_oper = {};

int main(int argc,char * argv[])
{
    return fuse_main(argc, argv, &fext2_oper, NULL);
}