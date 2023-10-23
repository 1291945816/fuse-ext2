#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "fuse-ext2/fext2.h"
#include "fuse-ext2/types.h"
#include "utils.h"

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
    // 根据buffer offset 
    struct fext2_dir_entry * entry = (struct fext2_dir_entry *)buffer;
    uint16_t a_size = entry->rec_len; // 实际长度
    uint16_t block_offset = 0; // 块内偏移
    
    /*根据目录大小来判断迭代次数[可能没有什么作用！]*/
    while(a_size <= dir->i_size ) {
        
        /*找到*/
        if (strcmp(entry->file_name, name) == 0)
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
        if (block_number >= dir->i_blocks)
            goto out_result;

         // 读取新的块（块内偏移不够存储一个目录项时 需要到下一个块获取）
         // TODO: 值得优化[261是 6+256 -1 文件块]
        if ((BLOCK_SIZE - block_offset) < sizeof(struct fext2_dir_entry)) {
            
            memset(buffer, 0, sizeof(uint8_t)*BLOCK_SIZE); // 置0
            read_inode_data_block(buffer, block_number++, dir);
            entry = (struct fext2_dir_entry *)buffer;
            block_offset = 0;
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
    // 考虑边界情况 最后一页 最后一个
    if (cur_entry_data->block_number == dir->i_blocks-1 && 
       (BLOCK_SIZE - cur_entry_data->offset) < sizeof(struct fext2_dir_entry)) 
    {
        return NULL;
    }
    else if ( cur_entry_data->block_number != dir->i_blocks-1 &&
      (BLOCK_SIZE - cur_entry_data->offset) < sizeof(struct fext2_dir_entry)) /*最后一个 但有下一页*/
    {

    
    }
    else // 其他情况
    {
    
    }




    return NULL;
}


