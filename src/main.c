#include "common.h"
#include "debug.h"
#include "device.h"
#include "fuse-ext2/fext2.h"

/*注册函数*/
static struct fuse_operations fext2_oper = {
    .init    = fext2_init, // 初始化
    .destroy = fext2_destory,
    .statfs  = fext2_statfs,
    .getattr = fext2_getattr,
    .opendir = fext2_opendir,
    .readdir = fext2_readdir,
    .mkdir   = fext2_mkdir,
    .rmdir   = fext2_rmdir
    
};

int main(int argc,char * argv[])
{
    


    // 需要先打开设备文件
    int i = 1;
    for (;i < argc;++i)
        if (memcmp(argv[i], "--mount_image", sizeof("--mount_image")) == 0) 
            break;
    if (i == argc) {
        DBG_PRINT("you need to add mount_image by `--mount_image`!");
        return -1;
    }
    else  // 尝试打开设备
    {
        
        if (!device_open(argv[i+1]))
        {
            DBG_PRINT("can't open the %s!",argv[i+1]);
            return -1;  
        }
        else
        {
            for (; i < argc-2; ++i) {
                argv[i] = argv[i+2]; // xx xxx xxx 
            }
            argc -= 2;
        };
    }
    return fuse_main(argc, argv, &fext2_oper, NULL);
}


