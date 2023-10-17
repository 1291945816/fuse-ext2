
#include"bitmap.h"
#include "fuse-ext2/fext2.h"
/**
 * @brief Get the zero bit object
 *  默认数组大小为BLOCK_SIZE 
 * @param buffer 
 * @return int -1 表示出问题 
 */
int get_zero_bit(uint8_t * buffer)
{
    
    // 每组有多少个块号是固定的
    for (uint32_t i = 0; i < BLOCK_SIZE; i++)
    {
        // 每一个字节 判断它是否是空的？
        uint8_t tmp = buffer[i];
        if (tmp == 255)
            continue;
        for (uint8_t j = 0; j < 8; j++)
        {
            uint8_t result= (uint8_t)(tmp >> j) & 1;
            if (result == 0)
                return i*8+j;   
        }
    }
    return -1;
}


/**
 * @brief 置状态位为1
 * 
 * @param buffer 
 * @param index 代表 组内的索引号 确保其小于8192
 * 
 */

void  bitmap_set(uint8_t * buffer,uint32_t index)
{
    if (index >= BLOCK_SIZE*8)
        return;

    int i = index / 8; 
    int j = index % 8; 
    uint8_t tmp=buffer[i];
    tmp |= (1 << j);
    buffer[i] = tmp;
}

/**
 * @brief 清除状态位 即 置 0
 * 
 * @param buffer 
 * @param index 代表 组内的索引号 
 * 
 */
void  bitmap_clear(uint8_t * buffer,uint32_t index)
{
    if (index >= BLOCK_SIZE*8)
        return;
    int i = index / 8; 
    int j = index % 8; 
    uint8_t tmp=buffer[i];
    tmp &= ~(1 << j);
    buffer[i] = tmp;
}


