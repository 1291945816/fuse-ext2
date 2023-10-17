#ifndef INCLUDE_FUSE_EXT2_FEXT2_INIT_T
#define INCLUDE_FUSE_EXT2_FEXT2_INIT_T
#include"types.h"
#include"../device.h"
#include"fext2.h"



/**
 * @brief 
 * 初始文件系统的超级块、组描述符号表
 * ！仅执行一次用以初始化最新的数据
 * @return int 
 */
int init_meta_info();

/**
 * @brief 擦除磁盘的数据
 * 
 */
void erase_disk();




#endif