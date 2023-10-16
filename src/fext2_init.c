#include<string.h>
#include<stdio.h>
#include<stdlib.h>

#include "fuse-ext2/fext2_init.h"
#include "device.h"
#include "debug.h"
int init_meta_info()
{
    /*super block*/
     strcpy(fext2_sb.s_volume_name,"fuse-ext2");
     fext2_sb.s_magic = FILESYSTEM_MAGIC;
     fext2_sb.s_disk_size = DISK_SIZE;
     fext2_sb.s_blocks_per_group = BLOCKS_PER_GROUP;
     fext2_sb.s_inodes_per_group = INODES_PER_GROUP;
     fext2_sb.s_blocks_size = BLOCK_SIZE;
     fext2_sb.s_inode_size = sizeof(struct fext2_inode); // 按字节计
     fext2_sb.s_blocks_count = DISK_SIZE;
     fext2_sb.s_inodes_count = INODES_SIZE;
     fext2_sb.s_mtime = 0;
     fext2_sb.s_state =0;
     



    uint32_t groups_nums = NUM_GROUP;

     DBG_PRINT("The number of groups in the file system: %lu",groups_nums);

     fext2_groups_table = (struct fext2_group_desc *)malloc(sizeof(struct fext2_group_desc)*(groups_nums));

     
   
     for (size_t i = 0; i < groups_nums; i++)
     {

        fext2_groups_table[i].bg_block_bitmap = i * BLOCKS_PER_GROUP + 5;
        fext2_groups_table[i].bg_inode_bitmap = i * BLOCKS_PER_GROUP + 6;
        fext2_groups_table[i].bg_inode_table  = i * BLOCKS_PER_GROUP + 7;
        // 7代表的是 1块存储超级块 4块存储组描述符 2块存储 位图 1+2+4 再减去inode表
        fext2_groups_table[i].bg_free_blocks_count = BLOCKS_PER_GROUP -(INODES_PER_GROUP/(BLOCK_SIZE/sizeof(struct fext2_inode)))- 7;
        fext2_groups_table[i].bg_free_inodes_count = INODES_PER_GROUP;
        fext2_groups_table[i].bg_used_dirs_count = 0;
     }
    
     // TODO: 支持部分备份 而不是全部备份
     // 备份数据 超级块、组描述符号（针对每一个块都如此操作）
     for (size_t i = 0; i < groups_nums; i++)
     {

        device_seek(i*BLOCK_SIZE * BLOCKS_PER_GROUP);
        device_write(&fext2_sb,sizeof(struct fext2_super_block));

        // (i*BLOCK_SIZE * BLOCKS_PER_GROUP) 移动块头了 超级块仅占一块
        device_seek((i*BLOCK_SIZE * BLOCKS_PER_GROUP)+BLOCK_SIZE);
        device_write(fext2_groups_table,sizeof(struct fext2_group_desc)*(groups_nums));
     }

     // 释放对应的内容
     free(fext2_groups_table);
     // 置为空指针
     fext2_groups_table = NULL;
}


void erase_disk()
{
    
    char Buffer[BLOCK_SIZE]={0};
    for (size_t i = 0; i < DISK_SIZE; i++)
    {
        device_seek(i*BLOCK_SIZE);
        device_write(Buffer,BLOCK_SIZE);
    }

}




