#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>


#include "fuse-ext2/fext2.h"
#include "device.h"
#include"bitmap.h"
#include "fuse-ext2/types.h"
#include "utils.h"

/**
 * @brief Get the unused inode object 
 * 获取没有使用的inode ino
 * @param group_number 
 * @return uint32_t 
 */
uint32_t get_unused_inode(uint32_t group_number)
{
    uint8_t buffer[BLOCK_SIZE];
    read_inode_bitmap(buffer,group_number);

    int index = get_zero_bit(buffer);

    //  默认 0 为不使用 表示 没有空的了
    if (index == -1)
        return 0;

    //比如组2 index 3 组2的起始ino号为 8192 * 2 + 3 若ino为0 默认+1 1 -> 0
    uint32_t ino = index + group_number * fext2_sb.s_inodes_per_group + 1;
    return ino;
}


/**
 * @brief 
 * 根据inode提供的状态，设置其位图上的状态
 * 同时会同步到磁盘中 而块组描述符号的数据并没同步到磁盘中
 * @param ino 
 * @param state 
 */
void inode_bitmap_set(uint32_t ino,uint8_t state)
{
    // 获取group号
    uint32_t group_number =  (ino-1) /  fext2_sb.s_inodes_per_group; 
    // 获取组内索引号
    uint32_t group_index = (ino-1) % fext2_sb.s_inodes_per_group;

    uint8_t buffer[BLOCK_SIZE];
    read_inode_bitmap(buffer,group_number);

    if (state == 1)
    {
        bitmap_set(buffer,group_index);
        fext2_groups_table[group_number].bg_free_inodes_count --;
    }    
    else
    {
        bitmap_clear(buffer,group_index);
        fext2_groups_table[group_number].bg_free_inodes_count ++;
    }

    // TODO:这些文件并不支持多线程操作
    // 直接更新位图数据
    device_seek(fext2_groups_table[group_number].bg_inode_bitmap*BLOCK_SIZE);
    device_write(buffer,BLOCK_SIZE);
    device_fflush();
}

/**
 * @brief 
 * 获取inode中某一数据块的内容
 * @param block 存储
 * @param data_block_index 数据块索引 inode 内索引 
 * @param inode 
 * @return Bool 
 */
Bool read_inode_data_block(void * block,uint32_t data_block_index, const struct fext2_inode  *  inode )
{
    
    if (data_block_index < FEXT2_N_BLOCKS-1) 
    {
        return read_data_block(block, inode->i_block[data_block_index]);
    }
    else // 处于间接块中
    {

        uint32_t tmp_block[BLOCK_SIZE/sizeof(uint32_t)];
        uint32_t offset = data_block_index - FEXT2_N_BLOCKS + 1;

        read_data_block(tmp_block, inode->i_block[FEXT2_N_BLOCKS-1]);
        
        // 从指定偏移获取块号 随后读取存储的地址所指向的块号
        read_data_block(block,tmp_block[offset]);
        // device_read_byte(tmp_block,sizeof(uint32_t),BLOCK_SIZE/sizeof(uint32_t));
    }
}