

#include "debug.h"
#include "device.h"
#include "fuse-ext2/fext2.h"
#include "fuse-ext2/types.h"
#include "utils.h"
#include <string.h>


/**
 * @brief 
 * 根据name 获取到实际的目录项 并返回所在块索引[方便获取它的前一个目录项以及后一个目录项]
 * @param dir 父目录名
 * @param name 要查找的文件or目录名
 * @param block_number 这是一个出参 返回当前块的块号 
 * @return struct fext2_dir_entry* 找到的目录项
 */
struct fext2_dir_entry * find_entry(struct fext2_inode * dir, const char * name,fext2_entry_helper * out_data)
{

    // 先获取dir的第一个块 按字节读取数据
    uint8_t buffer[BLOCK_SIZE]={0};
    uint16_t block_number = 0;
    read_inode_data_block(buffer, block_number++, dir);


    // TODO: 可改为do...while结构
    struct fext2_dir_entry * entry = (struct fext2_dir_entry *)buffer;
    uint16_t a_size = entry->rec_len; // 实际长度
    uint16_t block_offset = a_size; // 块内偏移长度 也即下一个file_entry的节点
    
    /*根据目录大小来判断迭代次数[可能没有什么作用！]*/
    while(a_size <= dir->i_size ) {
        
        /*找到 采用memcpy算法更优于strcpy  因为已经确保知道其长度*/
        if ((entry->name_len == strlen(name)) && memcmp(entry->file_name, name,strlen(name)) == 0)
        {
            struct fext2_dir_entry * ret_entry = (struct fext2_dir_entry * )malloc(sizeof(struct fext2_dir_entry));
            ret_entry->ino = entry->ino;
            ret_entry->file_type = entry->file_type;
            ret_entry->name_len = entry->name_len;
            ret_entry->rec_len = entry->rec_len;
            memcpy(ret_entry->file_name, entry->file_name, entry->name_len);

            /*传所在的块数据出去*/
            out_data->block_number = block_number-1;
            out_data->offset = block_offset;
            return ret_entry;
        }

        /*适当用goto*/
        if (((block_number < FEXT2_N_BLOCKS) && (block_number-1) >= dir->i_blocks) 
            // 考虑中间块
         )
            goto out_result;

         // 读取新的块（块内偏移不够存储一个目录项时 需要到下一个块获取）
         // TODO: 值得优化[261是 6+256 -1 文件块]
        if ((BLOCK_SIZE - block_offset) < sizeof(struct fext2_dir_entry)) {
            memset(buffer, 0, sizeof(uint8_t)*BLOCK_SIZE); // 置0
            read_inode_data_block(buffer, block_number++, dir);
            entry = (struct fext2_dir_entry *)buffer;
            block_offset = entry->rec_len;
            a_size += entry->rec_len;
            
        }else {

            entry = (struct fext2_dir_entry*)((void*)entry+entry->rec_len); // 获取到下一个[rec_len = sizeof(struct fext2_dir_entry)]
            /*没有这个条件可能会陷入死循环*/
            if (!entry->rec_len)
                goto out_result;
            a_size += entry->rec_len;
            block_offset += entry->rec_len;
            
        }
    }

out_result:
    out_data->block_number = 0;
    out_data->offset = 0;
    return NULL;
}

/**
 * @brief 
 * 根据当前目录项的信息获取上一个目录项的内容
 * @param dir 父目录
 * @param cur_entry_data 当前目录的数据 
 * @return struct fext2_dir_entry* 上一个目录项的内容
 */
struct fext2_dir_entry * previous_entry(struct fext2_inode * dir,const fext2_entry_helper * cur_entry_data)
{
    if (!cur_entry_data) 
        return NULL;
    
    uint8_t buffer[BLOCK_SIZE]={0};
    struct fext2_dir_entry * entry=NULL;
    // 处理边界情况 当偏移为0时
    /*块偏移*/
    if (cur_entry_data->block_number == 0 && cur_entry_data->offset == 0)
    {
        return NULL;
    }
    else if (cur_entry_data->block_number != 0 && cur_entry_data->offset == 0)/*考虑有前一个块 但是当前节点位于头部*/
    {
        read_inode_data_block(buffer, cur_entry_data->block_number - 1, dir);

        // 找到最后一个
        uint32_t block_offset = 0; // 设置块内偏移量
        entry = (struct fext2_dir_entry *)buffer;
        while ((BLOCK_SIZE - block_offset) > sizeof(struct fext2_dir_entry ) ) {
            entry = (struct fext2_dir_entry*)((void*)entry+entry->rec_len);
            block_offset += entry->rec_len;
        }
    }
    else 
    {
        // sizeof(struct fext2_dir_entry) 对后续的可变长度有影响 需要二次优化，现在默认是固定长度
        uint32_t previous_offset = cur_entry_data->offset - sizeof(struct fext2_dir_entry);
        read_inode_data_block(buffer, cur_entry_data->block_number, dir);
        entry = (struct fext2_dir_entry*)((void*)buffer+previous_offset);

    } // 其他情况都应该能在这块内找到数据 


    struct fext2_dir_entry * ret_entry = (struct fext2_dir_entry *) malloc(sizeof(struct fext2_dir_entry));
    memcpy(ret_entry->file_name,entry->file_name, entry->name_len);
    ret_entry->file_type = entry->file_type;
    ret_entry->name_len = entry->name_len;
    ret_entry->ino = entry->ino;
    ret_entry->rec_len = entry->rec_len;
    return ret_entry;
}


/**
 * @brief 
 * 根据当前目录项的信息获取下一个目录项的内容
 * @param dir 父目录
 * @param cur_entry_data 当前目录的数据 
 * @return struct fext2_dir_entry* 下一个目录项的内容
 */
struct fext2_dir_entry * next_entry(struct fext2_inode * dir,const fext2_entry_helper * cur_entry_data)
{

    if (!cur_entry_data) 
        return NULL;
    
    uint8_t buffer[BLOCK_SIZE]={0};
    struct fext2_dir_entry * entry=NULL;

    // 考虑边界情况 最后一页 最后一个
    if (cur_entry_data->block_number == dir->i_blocks-1 && 
       (BLOCK_SIZE - cur_entry_data->offset) < sizeof(struct fext2_dir_entry)) 
    {
        return NULL;
    }
    else if ( cur_entry_data->block_number != dir->i_blocks-1 &&
      (BLOCK_SIZE - cur_entry_data->offset) < sizeof(struct fext2_dir_entry)) /*最后一个 但有下一页*/
    {
        
        read_inode_data_block(buffer, cur_entry_data->block_number + 1, dir);
        // 找到第一个
        entry = (struct fext2_dir_entry *)buffer;
    }
    else // 其他情况
    {
        read_inode_data_block(buffer, cur_entry_data->block_number, dir);
        uint32_t next_offset = cur_entry_data->offset + sizeof(struct fext2_dir_entry);
        entry = (struct fext2_dir_entry*)((void*)buffer + next_offset);
    }

    struct fext2_dir_entry * ret_entry = (struct fext2_dir_entry *) malloc(sizeof(struct fext2_dir_entry));
    memcpy(ret_entry->file_name,entry->file_name, entry->name_len);
    ret_entry->file_type = entry->file_type;
    ret_entry->name_len = entry->name_len;
    ret_entry->ino = entry->ino;
    ret_entry->rec_len = entry->rec_len;
    return ret_entry;
}
/**
 * @brief 
 * 根据name（首字符 不能以 /开头，/结尾）
 * @param dir 
 * @param child 
 * @return uint32_t 
 */
uint32_t lookup_inode_by_name(struct fext2_inode *dir, const char *child)
{

    // / A/B/C  A B/C  B C C 分别拆解 /AB/
    // 获取目录前可以先移除了最后一个字符的/
    int index = find_char(child, '/');
    fext2_entry_helper help_data;
    if (index == -1) { // 最后一层目录

        struct fext2_dir_entry *entry = find_entry(dir, child, &help_data);
        if (entry == NULL) {
            DBG_PRINT("ERROR:  child: %s", child);
            return 0;
        }
        // struct fext2_inode* next_inode =read_inode(entry->ino);
        uint32_t ino = entry->ino;
        free(entry); // 释放对应的目录项
        return ino;
    } 
    else
    {
        char name[index + 1];
        memset(name, 0, index + 1); // 初始化为0
        memcpy(name, child, index); // 拷贝对应的字符

        struct fext2_dir_entry *entry = find_entry(dir, name, &help_data);
        if (entry == NULL) {
            return 0;
        }
        struct fext2_inode *next_inode = read_inode(entry->ino);
        free(entry); // 释放对应的目录项
        return lookup_inode_by_name(next_inode, child + index + 1);
    }

    return 0;
}



/**
 * @brief 
 * 根据提供的dir 添加子项
 * @param dir 
 * @param child_entry 
 */
Bool add_entry(uint32_t dir_ino,struct fext2_inode * dir,struct fext2_dir_entry * child_entry)
{
    // 初始页
    if (dir->i_size == 0) 
    {
        char buffer[BLOCK_SIZE]={0};

        // 拷贝指定大小的字节
        // buffer 默认指向数组元素的开头
        memcpy(buffer,child_entry, sizeof(struct fext2_dir_entry));
        write_data_blcok(buffer, dir->i_block[0]);
        dir->i_size += sizeof(struct fext2_dir_entry);
    }
    else // 找到最后一项 
    {
        uint32_t r_size = 0;
        uint32_t block_offset = 0;
        uint32_t block_number=0;
        uint8_t buffer[BLOCK_SIZE]= {0};
        
        uint32_t entry_fixed_size =  sizeof(struct fext2_dir_entry);


        read_inode_data_block(buffer, block_number++, dir);


        struct fext2_dir_entry * entry = (struct fext2_dir_entry *)buffer;
        r_size += sizeof(struct fext2_dir_entry);
        
        block_offset += entry->rec_len;  


        while ((r_size < dir->i_size) && entry->ino != 0) {

            // 先判断再找下一个 不满足条件就重新获取一个块
            // 这里的判断是剩余的空间是否满足存放下一个目录项
            if ((BLOCK_SIZE-block_offset) < sizeof(struct fext2_dir_entry)) {
                memset(buffer, 0, BLOCK_SIZE);
                read_inode_data_block(buffer, block_number++, dir);
            
                entry = (struct fext2_dir_entry *)buffer;
                r_size += entry_fixed_size;
                block_offset = entry->rec_len;            
            }
            else
            {
                entry = (struct fext2_dir_entry *)((void *)buffer + block_offset);
                block_offset += entry->rec_len;
                r_size += entry_fixed_size;
            }
        }


        DBG_PRINT("block_num %d i_blocks %d",block_number,dir->i_blocks);
        // 再验证一遍是否满足条件
        if ((BLOCK_SIZE - block_offset) < sizeof(struct fext2_dir_entry))
        {
            
            memset(buffer, 0, BLOCK_SIZE);

            if (block_number == dir->i_blocks) // 说明需要申请一个块来存储数据 
            {
                
                uint32_t group_number = (dir_ino-1)/fext2_sb.s_inodes_per_group;
                uint32_t block_no = get_unused_block(group_number);     // 获取信息的块号
                block_bitmap_set(block_no, 1); /*避免下面获取中间块的时候得到同一个*/
                DBG_PRINT("new block: %d",block_no);
                if(!wirte_ino_for_inode(block_no,dir,dir_ino))          // 写入ino到dir的block中
                {
                    block_bitmap_set(block_no, 0);
                    return FALSE;
                }
                dir->i_blocks ++;

            }
            else 
            {
                DBG_PRINT("current exec this.");
                read_inode_data_block(buffer, block_number, dir);
                 
            }
            memcpy((void *)buffer, child_entry, entry_fixed_size);
            write_inode_data_block(buffer, block_number, dir);
            dir->i_size += entry_fixed_size;
        }
        else
        {
            DBG_PRINT("blk_num %d  total %d",block_number-1,dir->i_blocks);
            memcpy((void *)buffer + block_offset, child_entry, entry_fixed_size);
            /*向块中写入数据*/
            Bool ret = write_inode_data_block(buffer, block_number-1, dir);
            if (ret == FALSE) 
            {
                DBG_PRINT("failure write");
                return ret;
            }
               
            dir->i_size += entry_fixed_size;
        }
    }

    return TRUE;    
}