#include "common.h"
#include "debug.h"
#include "device.h"
#include "utils.h"
#include "fuse-ext2/fext2.h"
#include "fuse-ext2/types.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cstdint>
#include <stdlib.h>
#include <string.h>









/**
 * @brief 文件系统的初始化
 * @param conn 
 * @return void* 
 */
void * fext2_init(struct fuse_conn_info *conn)
{
    DBG_PRINT("start initialize fext2 filesystem...\n");
    // 初始化超级块
    read_superblock();
    fext2_sb.s_state = 1;
    // 初始化块表
    read_group_desc();

    DBG_PRINT("successfully.\n");

}

/**
 * @brief 清除文件系统挂载信息以及在内存驻留的信息
 * 
 * @param data 
 */
void fext2_destory(void * data)
{
    fext2_sb.s_state = 0;
    // 同步数据
    update_superblock();
    update_group_desc();

    device_fflush();
    device_close(); // 销毁指针
    free(fext2_groups_table);
    DBG_PRINT("filesystem exited\n");
}

/**
 * @brief 
 * 获取文件系统的统计信息
 * @return int 
 */
int fext2_statfs(const char * path, struct statvfs * stat_vfs)
{
    uint32_t blk_free = 0,ino_free = 0;
    for (size_t i = 0; i < NUM_GROUP; i++) 
    {
        blk_free += fext2_groups_table[i].bg_free_blocks_count;
        ino_free += fext2_groups_table[i].bg_free_inodes_count;
    
    }
    stat_vfs->f_bfree = blk_free;    // 空闲块数目
    stat_vfs->f_bavail =blk_free;
    stat_vfs->f_frsize = 0;
    stat_vfs->f_blocks = fext2_sb.s_blocks_count;  
    stat_vfs->f_files = fext2_sb.s_inodes_count;  // 总共inode数目
    stat_vfs->f_ffree =ino_free;   // 空闲inode数目
    stat_vfs->f_favail =ino_free;
    stat_vfs->f_fsid = fext2_sb.s_magic;
    stat_vfs->f_bsize = BLOCK_SIZE;
    stat_vfs->f_namemax = FEXT2_MAX_NAME_LEN;

    DBG_PRINT("blk_free: %lu\t total_block: %lu",stat_vfs->f_bfree, DISK_SIZE);
    
    return 0;
}


/**
 * @brief 
 * 获取文件属性
 * @return int 
 */
int fext2_getattr(const char * path, struct stat * stabuf)
{
    DBG_PRINT("current path is: %s", path);

    uint32_t len = strlen(path);

    struct fext2_inode * root_inode= read_inode(INODE_ROOT_INO);
    if (root_inode == NULL) {
        DBG_PRINT("No root dir!");
        return -ENOENT;
    }
    

    if (len ==  1 && path[0] == '/') 
    {
        stabuf->st_ino = INODE_ROOT_INO; // stabuf->st_ino： The 'st_ino' field is ignored except if the 'use_ino' mount option is given
        stabuf->st_atime = root_inode->i_atime;
        stabuf->st_size = real_block(root_inode->i_blocks) * BLOCK_SIZE;
        stabuf->st_ctime = root_inode->i_ctime;
        stabuf->st_gid = root_inode->i_gid;
        stabuf->st_uid = root_inode->i_uid;
        stabuf->st_mode =root_inode->i_mode;
        stabuf->st_mtime = root_inode->i_mtime;
        stabuf->st_blksize = BLOCK_SIZE;
        stabuf->st_blocks = real_block(root_inode->i_blocks);
        stabuf->st_nlink = root_inode->i_link_count;
        

    }
    else
    {
        char child_path[len];
        strncpy(child_path, path+1, len); // 移除/ 要单独处理根目录的情况
        child_path[len-1] = '\0'; // 加结尾符
        uint32_t ino=lookup_inode_by_name(root_inode,child_path);
        if (ino == 0) {
            free(root_inode);
            return -ENOENT;
        }

        struct fext2_inode * cur_inode = read_inode(ino);
        stabuf->st_ino = ino;
        stabuf->st_atime = cur_inode->i_atime;
        stabuf->st_size = real_block(cur_inode->i_blocks) * BLOCK_SIZE;
        stabuf->st_ctime = cur_inode->i_ctime;
        stabuf->st_gid = cur_inode->i_gid;
        stabuf->st_uid = cur_inode->i_uid; 
        stabuf->st_mode =cur_inode->i_mode;
        stabuf->st_mtime = cur_inode->i_mtime;
        stabuf->st_blksize = BLOCK_SIZE;
        stabuf->st_blocks = real_block(cur_inode->i_blocks);
        stabuf->st_nlink = cur_inode->i_link_count;
        free(cur_inode);

    }
    

    free(root_inode);

    return 0;
}

/**
 * @brief 打开目录，并传递打开的信息
 * 
 * @param path 
 * @param file_info 
 * @return int 
 */
int fext2_opendir(const char * path, struct fuse_file_info * file_info)
{
    DBG_PRINT("opendir path: %s",path);

    uint32_t len = strlen(path);

    if ((len != 1 && path[0] != '/') || (len == 1 && path[0] != '/')) // 操作不允许
    {
        return -EPERM;
    }

    char path_copy[len];  
    strncpy(path_copy, path+1, len); // 移除/ 要单独处理根目录的情况
    path_copy[len-1] = '\0'; // 加结尾符

    
    struct fext2_inode * root_inode = read_inode(INODE_ROOT_INO);
    if (root_inode == NULL) {
        DBG_PRINT("No root dir!");
        return -ENOENT;
    }

    if (len == 1 && path[0] == '/') {
        file_info->fh = INODE_ROOT_INO;  //单独处理根目录的情况
        free(root_inode);
        return 0;
    }


    // 根据路径查询节点 并获取ino
    uint32_t ino=lookup_inode_by_name(root_inode,path_copy);

    if (ino == 0) {
        DBG_PRINT("No this Directory!");
        return -ENOENT;
    }
    struct fext2_inode * ret = read_inode(ino);

    if (IS_DIR(ret->i_mode))
    {
        file_info->fh =ino; // 文件指针 指向ino
        free(ret);
        free(root_inode);
        return 0;
    }else
    {
        DBG_PRINT("No this Directory!");
        free(ret);
        free(root_inode);
        return -ENOTDIR;
    }
    free(ret);
    return -ENOENT;
}


/**
 * @brief 打开一个目录
 * 该方法有两种模式 1.利用偏移读取下一个目录 2.一次性读取全部目录（采取这种方案）
 * @return int 
 */
int fext2_readdir(const char * path, void * buff, 
                        fuse_fill_dir_t filler,
                        off_t offset,
                        struct fuse_file_info * file_info)
{
    uint8_t block[BLOCK_SIZE]={0};
    uint32_t curr_blk = 0;
    uint32_t curr_size = 0;
    uint32_t blk_offset = 0;



    // 获取打开的目录的信息
    uint32_t ino = (uint32_t)file_info->fh;
    struct fext2_inode *dir_inode= read_inode(ino);
    DBG_PRINT("current_path is %s", path);
    DBG_PRINT("dir size is %.2lf", (dir_inode->i_size/(1024*1.0)));

    filler(buff,".",NULL,0); // 当前目录
    filler(buff,"..",NULL,0); // 上一层目录



    while (dir_inode->i_size > 0 && (read_inode_data_block(block,curr_blk++,dir_inode))==FALSE){
        memset(block, 0, sizeof(uint8_t)*BLOCK_SIZE);
    };

    struct fext2_dir_entry * entry= (struct fext2_dir_entry *)block;
    blk_offset  += entry->rec_len;
    curr_size += sizeof(*entry);

    while (curr_size <= dir_inode->i_size)
    {

        char name[FEXT2_MAX_NAME_LEN+1]={0};
        memcpy(name, entry->file_name, entry->name_len);
        name[entry->name_len] = '\0';
        filler(buff,name,NULL,0); // 填充数据

        DBG_PRINT("reading: %s,total %lu Byte", name,sizeof(*entry));
        DBG_PRINT("current_ino: %d", entry->ino);
        DBG_PRINT("curr_size %d  dir_inode->i_size %d",curr_size,dir_inode->i_size);
        
        if ((BLOCK_SIZE - blk_offset) < sizeof(struct fext2_dir_entry))
        {
            DBG_PRINT("cur_blk: %d",curr_blk);
            memset(block, 0, sizeof(uint8_t)*BLOCK_SIZE);
            while (curr_size < dir_inode->i_size && (read_inode_data_block(block,curr_blk++,dir_inode))==FALSE)
            {
                memset(block, 0, sizeof(uint8_t)*BLOCK_SIZE);
            }
            entry= (struct fext2_dir_entry *)block;
            blk_offset = entry->rec_len;
            curr_size += sizeof(*entry);
        }
        else
        {
            entry = (struct fext2_dir_entry *)((void *)entry + entry->rec_len); // 到下一项

            if (entry->ino == 0) 
            {
                 DBG_PRINT("current_ino: 0");
                // ******针对反复利用的页进行的处理******************
                 if (curr_size <= dir_inode->i_size)
                 {
                    DBG_PRINT("cur_blk: %d",curr_blk);
                    memset(block, 0, sizeof(uint8_t)*BLOCK_SIZE);
                    while (curr_size < dir_inode->i_size && (read_inode_data_block(block,curr_blk++,dir_inode))==FALSE)
                    {
                        memset(block, 0, sizeof(uint8_t)*BLOCK_SIZE);
                    }
                    entry= (struct fext2_dir_entry *)block;
                    blk_offset = entry->rec_len;
                    curr_size += sizeof(*entry);
                 }else
                {
                    break;
                }
  
            }else 
            {
                blk_offset  += entry->rec_len;
                curr_size += sizeof(*entry);          
            }

        }        
    }
    free(dir_inode);

    return 0;
}


/**
 * @brief
 * 创建一个目录
 * @return int 
 */
int fext2_mkdir(const char * path, mode_t mode)
{
    struct fuse_context *fcxt=fuse_get_context();
    uint32_t group_number = 0;
    uint32_t ino = 0;
    uint32_t parent_ino = 0;
    uint32_t blk_ino = 0;
    struct fext2_inode *parent_dir = NULL;
    const uint32_t len=strlen(path);

    // 解析父目录 & 当前目录
    char * parent_path = (char*)malloc(sizeof(char)* len+1); // 记得释放空间
    memset(parent_path, '\0', len+1);
    char dir_name[FEXT2_MAX_NAME_LEN]={0};

    parse_cur_dir(path, parent_path, dir_name);

    // 获取目录
    struct fext2_inode * root_dir = read_inode(INODE_ROOT_INO);
    // 根路径后接的
    if (!strlen(parent_path) && strlen(dir_name)) 
    {
        DBG_PRINT("parent dir is %s \t current dir is %s", "/", dir_name); 
        parent_ino = INODE_ROOT_INO;   
        parent_dir = root_dir;
    }
    else
    {
        free(root_dir);
        DBG_PRINT("parent dir is %s \t current dir is %s", parent_path, dir_name);
        parent_ino = lookup_inode_by_name(root_dir, parent_path+1); // 不准以/ 开头 故parent+1 跳过/
        if (!parent_ino)
        {
            free(parent_path);
            return -ENOENT;

        }
            
        parent_dir = read_inode(parent_ino);
    }
    
    //尽量和父目录在同一个块组里面
    group_number = GET_GROUP_N(parent_ino);

    while ((ino=get_unused_inode(group_number)) == 0)
    {
        if (group_number >= NUM_GROUP)
            break;
        ++group_number;
    }
    DBG_PRINT("new ino: %u", ino);
    if (!ino) 
    {
        DBG_PRINT("[inode]DISK have not free inode!");
        free(parent_path);
        return -1;
    }

    DBG_PRINT("mode is %u", mode);
    group_number = 0;                                       /*从0开始递增*/
    while ((blk_ino=get_unused_block(group_number)) == 0)
    {
        if (group_number >= NUM_GROUP)
            break;
        ++group_number;
    }
    if (!blk_ino) 
    {
        DBG_PRINT("[data]DISK have not free data block!");
        free(parent_path);
        return -1;
    }

    struct fext2_inode new_inode;
    new_inode.i_uid = fcxt->uid;
    new_inode.i_gid = fcxt->gid;
    new_inode.i_atime = new_inode.i_ctime = new_inode.i_mtime = time(NULL);
    new_inode.i_dtime = 0;
    new_inode.i_block[0] = blk_ino;
    new_inode.i_blocks=1;
    new_inode.i_mode = mode|__S_IFDIR; // 模式
    new_inode.i_link_count = 1;
    new_inode.i_size = 0;

    struct fext2_dir_entry new_entry;
    strncpy(new_entry.file_name, dir_name, strlen(dir_name)+1); /*加一个后缀0*/
    new_entry.file_name[strlen(dir_name)]='\0';
    new_entry.file_type = DIR;
    new_entry.name_len = strlen(dir_name);
    new_entry.ino = ino;
    new_entry.rec_len = sizeof(new_entry);

    inode_bitmap_set(ino, 1);
    block_bitmap_set(blk_ino, 1);

    // 同步磁盘 信息更新
    Bool ret = add_entry(parent_ino, parent_dir, &new_entry); // 添加目录项
    if (ret == FALSE)
    {
        inode_bitmap_set(ino, 0);
        block_bitmap_set(blk_ino, 0);
        DBG_PRINT("have not add entry for dir");
        free(parent_dir);
        free(parent_path);
        return -EPERM;
    }
        
    write_inode(&new_inode, ino);
    write_inode(parent_dir, parent_ino);
    update_group_desc();
    free(parent_dir);
    free(parent_path);

    return 0;
}

/**
 * @brief 
 * 删除一个目录
 * @return int 
 */
int fext2_rmdir(const char * path)
{
    DBG_PRINT("path: %s",path);

    uint32_t parent_ino=0;
    struct fext2_inode * parent_dir= NULL;
    const uint32_t len=strlen(path);
    char dir_name[FEXT2_MAX_NAME_LEN]={0};

    char * parent_path = (char*)malloc(sizeof(char)* len+1); 
    memset(parent_path, '\0', len+1);




    // 解析路径
    parse_cur_dir(path, parent_path, dir_name);
    
    struct fext2_inode * root_dir = read_inode(INODE_ROOT_INO);
    if (!strlen(parent_path) && strlen(dir_name)) 
    {
        DBG_PRINT("parent dir: %s \t current dir:%s", "/", dir_name); 
        parent_ino = INODE_ROOT_INO;   
        parent_dir = root_dir;
    }
    else
    {
        free(root_dir);
        DBG_PRINT("parent dir: %s \t current dir: %s", parent_path, dir_name);
        parent_ino = lookup_inode_by_name(root_dir, parent_path+1); // 不准以/ 开头 故parent+1 跳过/
        if (!parent_ino)
        {
            free(parent_path);
            return -ENOENT;
        }
            
        parent_dir = read_inode(parent_ino);
    }

    // 移除dir_name
    Bool ret = remove_entry(parent_dir,dir_name);

    if (ret==FALSE) {
        DBG_PRINT("Sorry,have not remove %s",dir_name);
        free(parent_path);
        return -1;
    }
    // 更新目录节点信息
    DBG_PRINT("update inode");
    write_inode(parent_dir, parent_ino); 
    update_group_desc();
    free(parent_path);
    free(root_dir);
    return 0;

}

/**
 * @brief 
 * 打开一个文件,获取其文件信息
 * @param path 文件路径
 * @param file_info 文件信息 
 * @return int 
 */

int fext2_open(const char * path, struct fuse_file_info * file_info)
{
    DBG_PRINT("open %s", path);

    uint32_t len = strlen(path);
    // len为1时 / 或者 xx都不行
    if ((len == 1) || (len != 1 &&  path[0]!= '/') )
        return -EPERM; // 无权限
    else
    {
        char * path_copy = (char *) malloc(sizeof(char) * len);
        memset(path_copy, 0, len);
        strncpy(path_copy, path+1, len-1); // 移除掉/ 总长为len 实际移动len-1个
        struct fext2_inode * root = read_inode(INODE_ROOT_INO);
        uint32_t ino = lookup_inode_by_name(root, path_copy);
        free(path_copy);
        free(root);
        struct fext2_inode * ret = read_inode(ino);
        // 没找到这个文件
        if (ret == NULL)
        {
            DBG_PRINT("");
            return -ENOENT;
        
        }
        // 是否为文件
        if (IS_REG(ret->i_mode))
        {
            file_info->fh = ino;
            free(ret);
            return 0;
        }
        else
        {
            DBG_PRINT("Is not a file");
            free(ret);
            return -EPERM;
        }
    }
}


