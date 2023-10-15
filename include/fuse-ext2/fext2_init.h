#ifndef INCLUDE_FUSE_EXT2_FEXT2_INIT_T
#define INCLUDE_FUSE_EXT2_FEXT2_INIT_T
#include"types.h"
#include"../device.h"
#define  FILESYSTEM_MAGIC     1509          // 文件系统幻数



#define BLOCK_SIZE            1024          // 块大小为 1024 B  按字节计数
#define DISK_SIZE             1048576       //    磁盘总共块数
#define BLOCKS_PER_GROUP      8192         //    每个块组的块数
#define INODES_PER_GROUP       8192          // 每个块组的inode数目
#define INODES_SIZE           1048576      // inode 总数
#define NUM_GROUP             (DISK_SIZE/BLOCKS_PER_GROUP)     // 块组数目






// 后续可以引入锁机制
static struct fext2_super_block fext2_sb; 
static struct fext2_group_desc *fext2_groups_table;    // ！文件系统退出时要记得移除内存


/**
 * @brief 
 * 初始文件系统的超级块、组描述符号表
 * ！仅执行一次用以初始化最新的数据
 * @return int 
 */
int init_meta_info();

/**
 * @brief 
 * 读取超级块
 * @return int 
 */
int read_superblock();

/**
 * @brief 
 * 读取组描述符表
 * @return int 
 */
int read_group_desc();


/**
 * @brief 擦除磁盘的数据
 * 
 */
void erase_disk();



#endif