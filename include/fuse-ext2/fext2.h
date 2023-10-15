#ifndef INCLUDE_FUSE_EXT2_FEXT2_H
#define INCLUDE_FUSE_EXT2_FEXT2_H


#include"types.h"


#define  FILESYSTEM_MAGIC     1509          // 文件系统幻数



#define BLOCK_SIZE            1024          // 块大小为 1024 B  按字节计数
#define DISK_SIZE             1048576       //    磁盘总共块数
#define BLOCKS_PER_GROUP      8192         //    每个块组的块数
#define INODES_PER_GROUP       8192          // 每个块组的inode数目
#define INODES_SIZE           1048576      // inode 总数
#define NUM_GROUP             (DISK_SIZE/BLOCKS_PER_GROUP)     // 块组数目



// 后续可以引入锁机制
// TODO:考虑数据同步性的问题（啥时候将数据同步到磁盘中）
static struct fext2_super_block fext2_sb; 
static struct fext2_group_desc *fext2_groups_table;    // ！文件系统退出时要记得移除内存

/**
 * @brief 
 * 读取超级块
 * @return int 
 */
int read_superblock();


/**
 * @brief 更新超级块的内容
 * 
 */
int update_superblock();



/**
 * @brief 
 * 读取组描述符表
 * 
 */
void read_group_desc();

/**
 * @brief 更新组描述符
 * 
 * @return int 更新状态[始终为1]
 */
int update_group_desc();




#endif