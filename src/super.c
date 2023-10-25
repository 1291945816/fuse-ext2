#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>


#include "fuse-ext2/fext2_init.h"
#include "fuse-ext2/fext2.h"
#include "fuse-ext2/types.h"
#include "device.h"
#include "debug.h"
#include "utils.h"

// static 函数会隐藏具体的内容
struct fext2_super_block fext2_sb; 
struct fext2_group_desc *fext2_groups_table;    // ！文件系统退出时要记得移除内存


int read_superblock()
{

    device_seek(0*BLOCK_SIZE);
    device_read(&fext2_sb,sizeof(struct fext2_super_block));
    DBG_PRINT("filesystem name: %s\n",fext2_sb.s_volume_name);   
    // 判断幻数来确保系统是否已经初始化了
    if (fext2_sb.s_magic != FILESYSTEM_MAGIC)
    {

        // 初始化文件系统的基本信息
        init_meta_info();
        device_seek(0*BLOCK_SIZE);
        // 这个在后面写会引发空指针
        device_read(&fext2_sb,sizeof(struct fext2_super_block)); 
        read_group_desc(); // 后续需要这个块组描述符号

        // /信息构建
        struct fext2_inode root_inode;

        for (size_t i = 0; i < FEXT2_N_BLOCKS; ++i)
            root_inode.i_block[i] = 0;
        root_inode.i_mode = 0755 | __S_IFDIR; //文件模式 读写什么的权限
        root_inode.i_uid = getuid();
        root_inode.i_gid = getgid();
        root_inode.i_size = 0; // 默认分配一个块的大小
        root_inode.i_atime = 0;
        root_inode.i_ctime = time(NULL); // 当前时间戳
        root_inode.i_mtime =time(NULL); // 当前时间戳
        root_inode.i_dtime = 0;
        root_inode.i_link_count = 1;  // 创建默认为1
        root_inode.i_block[0]=get_unused_block(0); // 生成一个块号
        DBG_PRINT("Root init data block => %d\n", root_inode.i_block[0]);
        root_inode.i_blocks = 1; // 被分配的块

     
        inode_bitmap_set(2, 1); // 根节点
        inode_bitmap_set(1, 1);
        block_bitmap_set(root_inode.i_block[0], 1);
        fext2_groups_table[0].bg_used_dirs_count++; // 用于目录的
        update_group_desc(); 

        uint8_t buffer[BLOCK_SIZE]={0};
        write_data_blcok(buffer, root_inode.i_block[0]);

        write_inode(&root_inode, 2); // 向磁盘写入inode信息 
        // 释放
        free(fext2_groups_table); // 后续再根据情况申请 也可以不释放
           
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