#include "utils.h"
#include "device.h"
#include "fuse-ext2/fext2.h"
#include "fuse-ext2/types.h"
#include <stddef.h>
#include <string.h>
/**
 * @brief 
 * 根据块号从数据块区域中读取块的内容
 * @param block 
 * @param block_number 绝对块号 都需要减去1 从1开始 
 */
Bool read_data_block(void * block,uint32_t block_number)
{

    device_seek((DATA_BLOCK_BASE_PER_GROUP + block_number-1) * BLOCK_SIZE);
    device_read(block, BLOCK_SIZE);
    return TRUE;
}


Bool write_data_blcok(void * block,uint32_t block_number)
{
    
    device_seek((DATA_BLOCK_BASE_PER_GROUP+block_number-1)*BLOCK_SIZE);
    device_write(block, BLOCK_SIZE);
    device_fflush();
}



/**
 * @brief 
 * 查找某个字符第一次出现的位置
 * @param src 原字符串
 * @param c 查找的字符
 * @return uint32_t 
 */
int  find_char(const char * src,char c)
{
    int len = strlen(src);
    for (size_t i = 0; i < len; ++i) {
        if (src[i] == '/') {
            return i; // 返回第一次出现 A/B/C -> 1
        }
    
    }
    return -1; // 没找到 -1


}