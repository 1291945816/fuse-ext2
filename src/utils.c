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

/**
 * @brief 
 * 对一个路径进行解析 找出父目录以及当前目录
 * @param path 
 * @param parent_dir 出参 不能为空
 * @param cur_dir 出参  不能为空
 */
int parse_cur_dir(const char * path,char * parent_dir,char * cur_dir)
{
    uint32_t len = strlen(path);
    uint32_t raw_len = len;
    if (path == NULL ||!len )
        return -1;

    
    while (--len)
        if (path[len] == '/')
            break;
    if (!len)
    {
        if (path[len]== '/')
            strncpy(cur_dir, path+1, strlen(path));
        else
            strncpy(cur_dir, path, strlen(path)+1);
        return 0;
    }

    // len 已经指向最后第一个/了 /A/B/CC len = 4 raw_len = 7
    strncpy(cur_dir, path + len+1, raw_len - len-1);
    strncpy(parent_dir, path, len);
    return 0;
}