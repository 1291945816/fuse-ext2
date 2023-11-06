#include "common.h"
#include "debug.h"
#include "device.h"
#include "utils.h"
#include "fuse-ext2/fext2.h"
#include "fuse-ext2/types.h"






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

        stabuf->st_atime = cur_inode->i_atime;
        stabuf->st_size = real_block(cur_inode->i_blocks) * BLOCK_SIZE;
        stabuf->st_ctime = cur_inode->i_ctime;
        stabuf->st_gid = cur_inode->i_gid;
        stabuf->st_uid = cur_inode->i_uid;
        stabuf->st_mode =cur_inode->i_mode;
        stabuf->st_mtime = root_inode->i_mtime;
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
    DBG_PRINT("open path: %s",path);

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
        printf("No this Directory!");
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
    uint32_t blk_size = 0;



    // 获取打开的目录的信息
    uint32_t ino = (uint32_t)file_info->fh;
    struct fext2_inode *dir_inode= read_inode(ino);

    DBG_PRINT("dir size is %.2lf", (dir_inode->i_size/(1024*1.0)));

    filler(buff,".",NULL,0); // 当前目录
    filler(buff,"..",NULL,0); // 上一层目录


    read_inode_data_block(block,curr_blk++,dir_inode);
    struct fext2_dir_entry * entry= (struct fext2_dir_entry *)block;

    while (curr_size < dir_inode->i_size)
    {

        if ((BLOCK_SIZE - blk_size) < sizeof(struct fext2_dir_entry)) 
        {

            memset(block, 0, sizeof(uint8_t)*BLOCK_SIZE); 
            read_inode_data_block(block,curr_blk++,dir_inode);
            entry= (struct fext2_dir_entry *)block;
            blk_size = 0;
        }
        if (entry->ino == 0)  // 读取到了空数据
            break;


    
        char name[FEXT2_MAX_NAME_LEN+1];
        memcpy(name, entry->file_name, entry->name_len);
        name[entry->name_len] = '\0';

        
        curr_size += entry->rec_len;
        blk_size  += entry->rec_len;
        

        filler(buff,name,NULL,0); // 填充数据

        DBG_PRINT("reading：%s ,total %d Byte", entry->file_name,entry->rec_len);

        entry = (struct fext2_dir_entry *)((void *)entry + entry->rec_len); // 移到下一项
        
    }
    free(dir_inode);

    return 0;
    
}