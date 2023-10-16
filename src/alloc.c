
#include<stdio.h>
#include "fuse-ext2/fext2.h"
#include "device.h"

void read_inode_bitmap(void * buffer, uint32_t group_number)
{
    // 保证输入的组数是小于总共数目的
    if (group_number >= NUM_GROUP)
    {
        fprintf(stderr,"offer group number > total!\n");
        return;
    }

    uint32_t block_number = fext2_groups_table[group_number].bg_inode_bitmap;
    
    // 块号是绝对的而不是相对块地址的
    device_seek(block_number*BLOCK_SIZE);
    device_read(buffer,BLOCK_SIZE);
}

void read_block_bitmap(void * buffer, uint32_t group_number)
{
    if (group_number >= NUM_GROUP)
    {
        fprintf(stderr,"offer group number > total!\n");
        return;
    }

    uint32_t block_number = fext2_groups_table[group_number].bg_block_bitmap;
    device_seek(block_number*BLOCK_SIZE);
    device_read(buffer,BLOCK_SIZE);

}