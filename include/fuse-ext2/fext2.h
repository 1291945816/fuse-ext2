#ifndef INCLUDE_FUSE_EXT2_FEXT2_H
#define INCLUDE_FUSE_EXT2_FEXT2_H

#include "common.h"
#include"types.h"
#include <stdint.h>


#define  FILESYSTEM_MAGIC     1509          // 文件系统幻数


#define BLOCK_SIZE                  1024                             // 块大小为 1024 B  按字节计数
#define DISK_SIZE                   1048576                       //    磁盘总共块数
#define BLOCKS_PER_GROUP            8192                          //    每个块组的块数
#define INODES_PER_GROUP            8192                             // 每个块组的inode数目
#define INODES_SIZE                 1048576                          // inode 总数
#define NUM_GROUP                   (DISK_SIZE/BLOCKS_PER_GROUP)     // 块组数目
#define DATA_BLOCK_BASE_PER_GROUP   519                            //   数据块起始地址 组内块号偏移

#define INODE_ROOT_INO       2            // 根目录的ino号定为2


// 后续可以引入锁机制
// TODO:考虑数据同步性的问题（啥时候将数据同步到磁盘中）
extern struct fext2_super_block fext2_sb; 
extern struct fext2_group_desc *fext2_groups_table;    // ！文件系统退出时要记得移除内存



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





void read_block_bitmap(void * buffer, uint32_t group_number);



void read_inode_bitmap(void * buffer, uint32_t group_number);
void inode_bitmap_set(uint32_t ino,uint8_t state);
void block_bitmap_set(uint32_t block_index,uint8_t state);

uint32_t get_unused_inode(uint32_t group_number);
uint32_t get_unused_block(uint32_t group_number);


struct fext2_inode *  read_inode(uint32_t ino);
uint32_t write_inode(const struct fext2_inode * inode,uint32_t ino);


Bool read_inode_data_block(void * block,uint32_t data_block_index, const  struct fext2_inode * inode );
Bool write_inode_data_block(void * block,uint32_t data_block_index, const  struct fext2_inode * inode);
int32_t wirte_ino_for_inode(uint32_t block_no, struct fext2_inode * inode,uint32_t ino);
Bool free_inode_data_block(uint32_t data_block_index,struct fext2_inode * inode);




struct fext2_dir_entry * find_entry(struct fext2_inode * dir, const char * name,fext2_entry_helper * out_data);
struct fext2_dir_entry * previous_entry(struct fext2_inode * dir,const fext2_entry_helper * dir_entry_data,fext2_entry_helper * out_data);
struct fext2_dir_entry * next_entry(struct fext2_inode * dir,const fext2_entry_helper * dir_entry_data,fext2_entry_helper * out_data);

uint32_t lookup_inode_by_name(struct fext2_inode * dir, const char * child);

Bool add_entry(uint32_t dir_ino, struct fext2_inode * dir,struct fext2_dir_entry * child_entry);
void update_entry(struct fext2_inode * dir,const fext2_entry_helper * cur_entry_data,struct fext2_dir_entry * entry);


Bool remove_entry(struct fext2_inode * dir,const char * entry_name);


/*文件系统操作*/
void*  fext2_init(struct fuse_conn_info *conn);            
void   fext2_destory(void * );
int    fext2_statfs(const char *, struct statvfs *);  

// TODO: 还未处理权限控制
int    fext2_getattr(const char *, struct stat *);                            
int    fext2_opendir(const char *, struct fuse_file_info *); 
int    fext2_readdir(const char *, void *, fuse_fill_dir_t, off_t,struct fuse_file_info *);
int    fext2_mkdir(const char *, mode_t);
int    fext2_rmdir(const char *);
int    fext2_open(const char *, struct fuse_file_info *);
/**
 * @brief 
 * 返回实际的块数目
 * 当超过8块时，
 */
#define real_block(blocks) \
 ((blocks) >= (FEXT2_N_BLOCKS) ? (blocks+1):(blocks))

#endif