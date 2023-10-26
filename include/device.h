/*
* 使用文件模拟磁盘 1GB大小 
*/

#ifndef INCLUDE_UTILS_DEVICE_H
#define INCLUDE_UTILS_DEVICE_H
#include "common.h"
#include "fuse-ext2/types.h"


static FILE * fp;  

/**
 * @brief 
 * 打开设备
 * @param file_path 
 * @return Bool 是否成功打开
 */
Bool device_open(const char * file_path);

/**
 * @brief 
 * 关闭设备
 * @return Bool 
 */
Bool device_close();


/**
 * @brief 
 * 移动文件指针到指定的偏移位置 按字节移动
 * @param offset 相对起始位置的偏移位置 
 */
Bool device_seek(uint32_t offset);

/**
 * @brief 
 * 向指定位置写入数据
 * @param buffer 缓冲区的数据
 * @param size 大小
 * @return Bool 
 */
Bool device_write(void * buffer,uint32_t size);

/**
 * @brief 从指定位置读取size个字节的数据到buffer中
 * 
 * @param buffer 存数据的缓冲区
 * @param size 要读取的数据大小
 * @return Bool 
 */
Bool device_read(void * buffer,uint32_t size);

/**
 * @brief 刷位于缓冲区的数据到指定位置
 * 
 */
void device_fflush();

/**
 * @brief 
 * 按指定字节读取数据
 * @param buffer 存储数据的缓冲区
 * @param read_byte 每次读取的字节数
 * @param size 要读取的大小
 * @return Bool 
 */
Bool device_read_byte(void * buffer,uint32_t size,uint32_t n);

















#endif