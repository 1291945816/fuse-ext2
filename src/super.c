#include<string.h>
#include<stdio.h>
#include<stdlib.h>

#include "fuse-ext2/fext2_init.h"
#include "fuse-ext2/fext2.h"
#include "debug.h"
#include "device.h"

// static 函数会隐藏具体的内容
struct fext2_super_block fext2_sb; 
struct fext2_group_desc *fext2_groups_table;    // ！文件系统退出时要记得移除内存


int read_superblock()
{

    device_seek(0*BLOCK_SIZE);
    device_read(&fext2_sb,sizeof(struct fext2_super_block));
    DBG_PRINT("filesystem name: %s\n",fext2_sb.s_volume_name);
    i = 100;    
    // // 判断幻数来确保系统是否已经初始化了
    if (fext2_sb.s_magic != FILESYSTEM_MAGIC)
    {
        // 初始化文件系统的基本信息
        init_meta_info();
    }
    
}


void read_group_desc()
{
    if (!fext2_groups_table)
        fext2_groups_table = (struct fext2_group_desc *)malloc(sizeof(struct fext2_group_desc) * NUM_GROUP);
    
    device_seek(BLOCK_SIZE);
    device_read(fext2_groups_table,NUM_GROUP*sizeof(struct fext2_group_desc));
}


int update_superblock()
{
    // 要完成所有数据备份数据的更新
    for (size_t i = 0; i < NUM_GROUP; i++)
     {

        device_seek(i*BLOCK_SIZE * BLOCKS_PER_GROUP);
        device_write(&fext2_sb,sizeof(struct fext2_super_block));
     }
     device_fflush();
     return 1;
}

int update_group_desc()
{
    for (size_t i = 0; i < NUM_GROUP; i++)
     {

        device_seek((i*BLOCK_SIZE * BLOCKS_PER_GROUP)+BLOCK_SIZE);
        device_write(fext2_groups_table,sizeof(struct fext2_group_desc)*(NUM_GROUP));
     }
     device_fflush();
     return 1;
}