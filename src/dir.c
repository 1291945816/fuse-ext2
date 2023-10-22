#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fuse-ext2/fext2.h"
#include "fuse-ext2/types.h"
#include "utils.h"


// 根据name 获取到实际的目录项
struct fext2_dir_entry * find_entry(struct fext2_inode * dir, const char * name)
{

    // 先获取dir的第一个块 按字节读取数据
    uint8_t buffer[BLOCK_SIZE];
    uint8_t block_number = 0;
    read_inode_data_block(buffer, block_number++, dir);

    // 根据buffer offset 
    struct fext2_dir_entry * entry = (struct fext2_dir_entry *)buffer;
    uint16_t a_size = entry->rec_len; // 实际长度
    uint16_t block_offset = 0; // 块内偏移
    
    while(a_size <= dir->i_size ) {
        
        block_offset += entry->rec_len;
        // 找到了
        if (strcmp(entry->file_name, name) == 0)
        {
            /*拷贝数据*/
            /*...*/
            return entry;
        }
                // 读取新的块（块内偏移不够存储一个目录项时 需要到下一个块获取）
        if (block_offset >0 && (BLOCK_SIZE - block_offset) < sizeof(struct fext2_dir_entry)) {
            
            memset(buffer, 0, sizeof(uint8_t)*BLOCK_SIZE); // 置0
            read_inode_data_block(buffer, block_number++, dir);
            entry = (struct fext2_dir_entry *)buffer;
            block_offset = 0;
            a_size += entry->rec_len;
            
        }else {

            entry = (struct fext2_dir_entry*)((void*)entry+entry->rec_len); // 获取到下一个[rec_len = sizeof(struct fext2_dir_entry)]
            a_size += entry->rec_len;
        }
        

        
    
    }

    return NULL;
}

