
#include <asm-generic/errno.h>
#include <stdint.h>
#include<stdio.h>
#include "bitmap.h"
#include "debug.h"
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
    // 块号是绝对的而不是相对块地址的【描述项提供的】
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

/**
 * @brief
 * 根据块号分配一个未使用的块
 * @param group_number 块组号
 * @return int32_t 返回数据块号id 注意 数据块号是从1开始的！ 
 */
uint32_t get_unused_block(uint32_t group_number)
{
    uint8_t buffer[BLOCK_SIZE]={0};
    read_block_bitmap(buffer, group_number);
    int32_t block_no = get_zero_bit(buffer);

    // 直接返回即可
    return block_no == -1 ? 0: (block_no+1 + group_number*fext2_sb.s_blocks_per_group);
}

/**
 * @brief 
 * 更新位图数据 
 * @param block_index 绝对编号 id 是从1开始的
 * @param state 位图状态
 */
void block_bitmap_set(uint32_t block_index,uint8_t state)
{
    // 获取位图 需要知道组号
    uint8_t buffer[BLOCK_SIZE]={0};
    uint32_t group_number = (block_index-1) / fext2_sb.s_blocks_per_group;
    uint32_t index = (block_index-1) % fext2_sb.s_blocks_per_group;

    read_block_bitmap(buffer, group_number);

    if (state) {
        // 置1
        bitmap_set(buffer, index);
        fext2_groups_table[group_number].bg_free_blocks_count --;
    }
    else {
        bitmap_clear(buffer, index);
        fext2_groups_table[group_number].bg_free_blocks_count ++;
    }


    //更新位图
    device_seek(fext2_groups_table[group_number].bg_block_bitmap * BLOCK_SIZE);
    device_write(buffer, BLOCK_SIZE);
    device_fflush();
}


