#include "utils.h"
#include "device.h"
#include "fuse-ext2/fext2.h"
#include "fuse-ext2/types.h"
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