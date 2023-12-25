

#include "debug.h"
#include "device.h"
#include "fuse-ext2/fext2.h"
#include "fuse-ext2/types.h"
#include "utils.h"
#include <asm-generic/errno.h>
#include <stdint.h>
#include <stdio.h>


void test_print(uint32_t block_number,const struct fext2_inode *inode)
{
    char buffer[BLOCK_SIZE]={0};
    read_inode_data_block(buffer,block_number,inode);
    
    struct fext2_dir_entry * next = (struct fext2_dir_entry *)buffer;
    for (uint32_t i=0; i < 3; ++i)
    {
        DBG_PRINT("ino: %u,entry name: %s,rec_len: %u", next->ino,next->file_name,next->rec_len);
        next = (struct fext2_dir_entry *)((void*)next+263);
    }

}



/**
 * @brief 
 * 根据name 获取到实际的目录项 并返回所在块索引[方便获取它的前一个目录项以及后一个目录项]
 * @param dir 父目录名
 * @param name 要查找的文件or目录名
 * @param block_number 这是一个出参 返回当前块的块号 
 * @return struct fext2_dir_entry* 找到的目录项
 */
struct fext2_dir_entry * find_entry(struct fext2_inode * dir, 
                                    const char * name,
                                    fext2_entry_helper * out_data)
{

    if (!dir->i_size) 
    {
        DBG_PRINT("dir size is 0");
        return NULL;
    }   
    uint8_t buffer[BLOCK_SIZE]={0};
    uint32_t block_index = 0;
    while ((read_inode_data_block(buffer, block_index++, dir))== FALSE);
    

    struct fext2_dir_entry * entry = (struct fext2_dir_entry *)buffer;
    uint32_t cur_size = sizeof(*entry); 
    uint32_t block_offset = entry->rec_len; 



    while(cur_size <= dir->i_size ) {

        // DBG_PRINT("%d",dir->i_size);
        // DBG_PRINT("%d,%d,name: %s",cur_size,entry->ino,entry->file_name);
        
        /*已找到*/
        if (entry->ino != 0 && (entry->name_len == strlen(name)) && memcmp(entry->file_name, name,strlen(name)) == 0)
        {
            struct fext2_dir_entry * ret_entry = (struct fext2_dir_entry * )malloc(sizeof(struct fext2_dir_entry));
            ret_entry->ino = entry->ino;
            ret_entry->file_type = entry->file_type;
            ret_entry->name_len = entry->name_len;
            ret_entry->rec_len = entry->rec_len;
            memcpy(ret_entry->file_name, entry->file_name, entry->name_len);

            /*传所在的块数据出去*/
            out_data->block_number = block_index-1;
            out_data->offset = block_offset-entry->rec_len; // 已经指向下一个的开头  需要回退一个单位
            return ret_entry;
        }

         // 读取新的块（块内偏移不够存储一个目录项时 需要到下一个块获取）
         // TODO: 值得优化[261是 6+256 -1 文件块]
        if ( (BLOCK_SIZE - block_offset) < sizeof(struct fext2_dir_entry)) {
            memset(buffer, 0, sizeof(uint8_t)*BLOCK_SIZE); // 置0
            while ((block_index < FEXT2_MAX_BLOCKS) && ((read_inode_data_block(buffer, block_index++, dir))== FALSE));
            if ((block_index-1) < FEXT2_MAX_BLOCKS)
            {
                entry = (struct fext2_dir_entry *)buffer;
                block_offset = entry->rec_len;
                cur_size += sizeof(*entry);
            
            }else
            {
                break;          
            }
            
        }else {
            //  DBG_PRINT("test");
            entry = (struct fext2_dir_entry*)((void*)entry+entry->rec_len); // 获取到下一个[rec_len = sizeof(struct fext2_dir_entry)]
            if (entry->ino == 0 && cur_size < dir->i_size)
            {
                memset(buffer, 0, sizeof(uint8_t)*BLOCK_SIZE); // 置0
                while ( (block_index < FEXT2_MAX_BLOCKS) && ((read_inode_data_block(buffer, block_index++, dir))== FALSE))
                {
                    // DBG_PRINT("%d",block_index);
                    // DBG_PRINT("a_size: %d, dir_size: %d", cur_size,dir->i_size);
                }
                if (block_index-1 < FEXT2_MAX_BLOCKS) 
                {
                    entry = (struct fext2_dir_entry *)buffer;
                    block_offset = entry->rec_len;
                    cur_size += sizeof(*entry);
                }
                else
                {
                    goto out_result;
                }

            
            }
            else
            {
                cur_size += sizeof(*entry);
                block_offset += entry->rec_len;
                
            }


            
        }
    }

out_result:
    out_data->block_number = 0;
    out_data->offset = 0;
    return NULL;
}

/**
 * @brief 
 * 根据当前目录项的信息获取上一个目录项的内容【仅仅处理当前块】
 * @param dir 父目录
 * @param dir_entry_data 父目录的数据 
 * @return struct fext2_dir_entry* 上一个目录项的内容
 */
struct fext2_dir_entry * previous_entry(struct fext2_inode * dir,
                                        const fext2_entry_helper * dir_entry_data,
                                        fext2_entry_helper * out_data)
{
    if (!dir_entry_data) 
        return NULL;
    
    uint8_t buffer[BLOCK_SIZE]={0};
    struct fext2_dir_entry * entry=NULL;
    // 处理边界情况 当偏移为0时
    DBG_PRINT("dir_entry_data->offset: %u", dir_entry_data->offset);
    if (dir_entry_data->offset == 0)
    {
        return NULL;
    }
    else 
    {
        uint32_t previous_offset = 0;
        // 不会存在失败的情况
        read_inode_data_block(buffer, dir_entry_data->block_number, dir);
        // update
        entry = (struct fext2_dir_entry*)buffer;

        while (previous_offset+entry->rec_len < dir_entry_data->offset)
        {
            previous_offset +=entry->rec_len;
            entry = (struct fext2_dir_entry*)((void*)entry + entry->rec_len);
        }
        out_data->block_number = dir_entry_data->block_number;
        out_data->offset = previous_offset;

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
 * 根据当前目录项的信息获取下一个目录项的内容【仅仅处理当前块】
 * @param dir 父目录
 * @param cur_entry_data 当前目录的数据 
 * @return struct fext2_dir_entry* 下一个目录项的内容
 */
struct fext2_dir_entry * next_entry(struct fext2_inode * dir,
                                    const fext2_entry_helper * dir_entry_data,
                                    fext2_entry_helper * out_data)
{

    if (!dir_entry_data) 
        return NULL;
    
    uint8_t buffer[BLOCK_SIZE]={0};
    struct fext2_dir_entry * entry=NULL;
    read_inode_data_block(buffer, dir_entry_data->block_number, dir);
    uint32_t next_offset = dir_entry_data->offset;
        // 获取当前的内容
    entry = (struct fext2_dir_entry*)((void*)buffer + dir_entry_data->offset);
    next_offset += entry->rec_len;

    if (BLOCK_SIZE-next_offset < sizeof(struct fext2_dir_entry)) 
    {
        return NULL;
    }
    // 获取下一个
    entry = (struct fext2_dir_entry*)((void*)buffer + next_offset);
    if (entry->ino == 0)
    {
        return 0;
    }
    out_data->block_number = dir_entry_data->block_number;
    out_data->offset = next_offset;

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
            DBG_PRINT("child: %s", child);
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

        // 判断一下块是否是为 0 处理被释放的场景
        if (!dir->i_block[0])
        {
            uint32_t group_number = (dir_ino-1)/fext2_sb.s_inodes_per_group;
            uint32_t block_no = get_unused_block(group_number);     // 获取信息的块号
            dir->i_block[0] = block_no;
            block_bitmap_set(block_no, 1); /*避免下面获取中间块的时候得到同一个*/
            DBG_PRINT("new block: %d,index: %d",block_no,0);
            dir->i_blocks ++;
        }
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
        uint32_t block_index=0; // 块索引
        uint32_t block_num = 0; // 当前引入的块数目
        uint8_t buffer[BLOCK_SIZE]= {0};
        
        uint32_t entry_fixed_size =  sizeof(struct fext2_dir_entry);

        while ((read_inode_data_block(buffer, block_index++, dir))== FALSE);
        ++block_num;

        struct fext2_dir_entry * entry = (struct fext2_dir_entry *)buffer;
        r_size += sizeof(struct fext2_dir_entry);
        
        block_offset += entry->rec_len;  

        // 这里的逻辑还是存在一些问题
        while ((r_size < dir->i_size) && entry->ino != 0) {


            if ((BLOCK_SIZE-block_offset) < sizeof(struct fext2_dir_entry)) {
                memset(buffer, 0, BLOCK_SIZE);
                while ((read_inode_data_block(buffer, block_index++, dir))== FALSE);
                ++block_num;
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


        DBG_PRINT("block_num %d i_blocks %d",block_num,dir->i_blocks);
        // 再验证一遍是否满足条件
        if ((BLOCK_SIZE - block_offset) < sizeof(struct fext2_dir_entry))
        {
            
            memset(buffer, 0, BLOCK_SIZE);

            if (block_num == dir->i_blocks) // 说明需要申请一个块来存储数据 
            {
                
                uint32_t group_number = (dir_ino-1)/fext2_sb.s_inodes_per_group;
                uint32_t block_no = get_unused_block(group_number);     // 获取信息的块号
                if (block_no == 0)
                {
                    DBG_PRINT("this block group[%d] have no space",group_number);
                    return FALSE;
                }

                block_bitmap_set(block_no, 1); /*避免下面获取中间块的时候得到同一个*/
                if( (block_index = wirte_ino_for_inode(block_no,dir,dir_ino)) == -1)          // 写入ino到dir的block中
                {
                    block_bitmap_set(block_no, 0);
                    return FALSE;
                }
                DBG_PRINT("new block: %d,index: %d",block_no,block_index);
                dir->i_blocks ++;
            }
            else 
            {
                DBG_PRINT("current exec this.");
                read_inode_data_block(buffer, block_index, dir);
                 
            }
            memcpy((void *)buffer, child_entry, entry_fixed_size);
            write_inode_data_block(buffer, block_index, dir);
            dir->i_size += entry_fixed_size;
        }
        else
        {
            DBG_PRINT("blk_num %d  total %d",block_index-1,dir->i_blocks);
            memcpy((void *)buffer + block_offset, child_entry, entry_fixed_size);
            /*向块中写入数据*/
            Bool ret = write_inode_data_block(buffer, block_index-1, dir);
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
// 更新当前的update信息
void update_entry(struct fext2_inode * dir,
                  const fext2_entry_helper * cur_entry_data,
                  struct fext2_dir_entry * entry)
{
    uint8_t buffer[BLOCK_SIZE]={0};
    Bool ret = read_inode_data_block(buffer,cur_entry_data->block_number,dir);
    DBG_PRINT("update result: %d", ret);
    struct fext2_dir_entry * disk_entry = (struct fext2_dir_entry*)((void*)buffer + cur_entry_data->offset);
    
    disk_entry->rec_len = entry->rec_len;
    memcpy(disk_entry->file_name,entry->file_name,entry->name_len);
    disk_entry->file_type = entry->file_type;
    disk_entry->ino = entry->ino;
    disk_entry->name_len = entry->name_len;

    write_inode_data_block(buffer, cur_entry_data->block_number,dir); // 数据更新
}



/**
 * @brief 移除一个节点 同时释放占据的数据块
 * @param dir 父目录
 * @param entry_name 待移除的目录项名称 
 * @return Bool 
 */
Bool remove_entry(struct fext2_inode * dir,const char * entry_name)
{
    DBG_PRINT("remove entry name is %s", entry_name);
    struct fext2_entry_helper helper;
    
    
    struct fext2_dir_entry *entry = find_entry(dir, entry_name,&helper);
    if (entry == NULL)
        return FALSE;
    struct fext2_inode * inode = read_inode(entry->ino);

    // ***********************遍历释放占有的数据块*******************************
    uint8_t buffer[BLOCK_SIZE]={0};
    uint32_t total_blk=inode->i_blocks;

    if (inode->i_blocks > (FEXT2_N_BLOCKS-1)) {
        total_blk = FEXT2_N_BLOCKS-1;

        uint32_t tmp_block[BLOCK_SIZE/sizeof(uint32_t)];
        // 读出所有间接块的内容
        read_data_block(tmp_block, inode->i_block[FEXT2_N_BLOCKS-1]); 
        // 针对大于7的部分进行处理
        for (uint32_t t = FEXT2_N_BLOCKS-1; t < inode->i_blocks; ++t) {
            uint32_t cur_ino = tmp_block[t-FEXT2_N_BLOCKS+1];
            block_bitmap_set(cur_ino, 0);
            write_data_blcok(buffer, cur_ino); //移除数据
        }
        // 移除中间块
        block_bitmap_set(inode->i_block[FEXT2_N_BLOCKS-1], 0);
        write_data_blcok(buffer, inode->i_block[FEXT2_N_BLOCKS-1]); //移除数据

    }

    for (uint32_t i = 0; i < total_blk; ++i) {
        uint32_t cur_ino = inode->i_block[i];
        DBG_PRINT("cur_ino: %d", cur_ino);
        block_bitmap_set(cur_ino, 0);
        write_data_blcok(buffer, cur_ino); 
    }
    
    // 擦除位图信息
    inode_bitmap_set(entry->ino, 0);
    entry->ino = 0;  

    // ***********************处理目录项的内容*******************************
    struct fext2_entry_helper p_helper;
    struct fext2_dir_entry * p_entry = previous_entry(dir, &helper,&p_helper);

    if (p_entry == NULL) {
        DBG_PRINT("this is the first entry in data block.");
        // find  next
        struct fext2_entry_helper n_helper;
        struct fext2_dir_entry * next = next_entry(dir, &helper, &n_helper);

        if (next != NULL && n_helper.block_number == helper.block_number)
        {
            // 因为占了其位置 结合图进行分析 这里的处理影响蛮大的
            // dist (entry->rec_len - sizeof(*entry)) 
            next->rec_len += (entry->rec_len - sizeof(*entry))+ sizeof(*next); 
            update_entry(dir, &helper, next);
            update_entry(dir, &n_helper, entry);// 标0




            free(next);
        }
        else 
        {
            DBG_PRINT("free data block");
            free_inode_data_block(helper.block_number, dir);
            -- dir->i_blocks;
        }
            

    }
    else {

        // 在同一块内 有前一个目录项
        if (p_helper.block_number == helper.block_number)
        {
            DBG_PRINT("move next one via add next entry len");
            p_entry->rec_len += entry->rec_len;      
            update_entry(dir,&p_helper, p_entry); // 更新信息
            update_entry(dir,&helper, entry); // 更新entry的信息

        }
        else   
        {
            free_inode_data_block(helper.block_number, dir);
            -- dir->i_blocks;
        }
    }
    

    free(p_entry);
    free(entry);
    free(inode);
    
    
    dir->i_size -=  sizeof(*entry);
    return TRUE;


}

