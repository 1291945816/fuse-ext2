// 文件系统的类型定义 fext2_xxx类型

#ifndef INCLUDE_FUSE_EXT2_TYPES_H
#define INCLUDE_FUSE_EXT2_TYPES_H
#include "common.h"

#define R_ONLY 4
#define W_ONLY 2
#define X_ONLY 1


typedef enum Bool
{
    TRUE = 1,
    FALSE = 0
}Bool;

typedef enum FileType{
    REG =1,
    DIR = 2
}FileType;


/*TODO: 可以作为fext2的一部分 */
typedef struct fext2_entry_helper{
    uint32_t block_number;            // 所在块号[利用该块号获取数据]
    uint32_t offset;                  // 块内偏移
}fext2_entry_helper;




/**
 * 定义超级块 文件系统中仅有一份 而其他块组中则是实现备份
 * 占用大小 64B
 * 用于描述整个文件系统的基本信息
 * 是否按要求对齐无关紧要 一个块足够了
*/


struct fext2_super_block
{
    char       s_volume_name[16];              // 文件系统名称
    uint16_t   s_magic;                        // 文件系统幻数
    uint32_t   s_disk_size;                    // 磁盘大小
    uint32_t   s_inodes_count;                 // inode 总数
    uint32_t   s_blocks_count;                 // block 总数
    uint32_t   s_blocks_per_group;             // 每组的块数目
    uint32_t   s_inodes_per_group;             // 每组的inode数目
    uint32_t   s_blocks_size;                  // block 大小
    uint32_t   s_inode_size;                   // inode 大小
    uint32_t   s_mtime;                        // 文件系统挂载时间
    uint8_t    s_state;                        // 文件系统状态：挂载1/未挂载 0
    char       padding[13];                    // 填充
}__attribute__ ((packed));





// 组描述符
// 结构占用大小 32 B
// 这个需要保证都是 32B 这样
struct fext2_group_desc
{
    uint32_t   bg_block_bitmap;             // 块组中数据位图所在块号
    uint32_t   bg_inode_bitmap;             // 块组中inode位图所在块号
    uint32_t   bg_inode_table ;             // 块组中inode table所在块号（首块）
    uint16_t   bg_free_blocks_count;        // 块组中空闲的块数目
    uint16_t   bg_free_inodes_count;        // 块组中空闲的inode数目
    uint16_t   bg_used_dirs_count;          // 已分配的目录数目

    char       padding[14];                 // 填充
}__attribute__ ((packed)); // 保证按照最小字节对齐





#define FEXT2_N_BLOCKS     8               // 数据块数目 包含7个直接索引+1个间接索引
#define FEXT2_MAX_BLOCKS   262



// inode 索引节点
// 该文件系统不使用片
struct fext2_inode
{
    uint16_t  i_mode;                      //  文件模式 
    uint16_t  i_uid;                       //  拥有者的用户id 只用低16位
    uint16_t  i_gid;                       //  拥有组的组id  只用低16位
    uint32_t  i_size;                      //  文件大小 Byte
    uint32_t  i_atime;                     //  访问时间 最近一次访问时间
    uint32_t  i_ctime;                     //  创建时间 创建时间
    uint32_t  i_mtime;                     //  修改时间 最近一次修改时间
    uint32_t  i_dtime;                     //  删除时间 
    uint16_t  i_link_count;                //  链接计数
    uint16_t  i_blocks;                    //  块计数

    uint32_t  i_block[FEXT2_N_BLOCKS];     //  数据块指针 能够直接访问的地址+一次间接块指针+二次间接块指针+三次间接块指针（当前暂不启用后两个的功能）
                                           // 单个文件大小为 7KB+256KB
    char padding[2];
    
}__attribute__ ((packed));


// 定义文件名的长度[Ext2 规定的最大文件的]

#define FEXT2_MAX_NAME_LEN    255

// 同一级的目录都会存在一个块中 所以找同一级的所有的文件or目录的时候都可以通过base+rec_len进行处理
// TODO:在ext2文件系统中 该项必须是4的倍数
struct fext2_dir_entry
{
    uint32_t ino;                        // inode 号 ino更具辨识度
    uint16_t rec_len;                      // 目录项长度 [用于可变长度]
    uint8_t  name_len;                     // 名字长度
    uint8_t  file_type;                    // 文件类型 1. 常规文件  2. 目录文件
    char     file_name[FEXT2_MAX_NAME_LEN];// 可以使用可变长度的数组（暂时不用,现在是固定大小的）

};


#endif