

#include "common.h"

#include "fuse-ext2/fext2.h"
#include "device.h"
#include "bitmap.h"
#include "fuse-ext2/types.h"
#include "utils.h"
#include "debug.h"

/**
 * @brief Get the unused inode object 
 * 获取没有使用的inode ino
 * @param group_number 
 * @return uint32_t 0 表示没有空余空间了
 */
uint32_t get_unused_inode(uint32_t group_number)
{
    uint8_t buffer[BLOCK_SIZE];
    read_inode_bitmap(buffer,group_number);

    int index = get_zero_bit(buffer);

    //  默认 0 为不使用 表示 没有空的了
    if (index == -1)
        return 0;

    //比如组2 index 3 组2的起始ino号为 8192 * 2 + 3 若ino为0 默认+1 1 -> 0
    uint32_t ino = index + group_number * fext2_sb.s_inodes_per_group + 1;
    return ino;
}


/**
 * @brief 
 * 根据inode提供的状态，设置其位图上的状态
 * 同时会同步到磁盘中 而块组描述符号的数据并没同步到磁盘中
 * @param ino 
 * @param state 
 */
void inode_bitmap_set(uint32_t ino,uint8_t state)
{
    // 获取group号
    uint32_t group_number =  (ino-1) /  fext2_sb.s_inodes_per_group; 
    // 获取组内索引号
    uint32_t group_index = (ino-1) % fext2_sb.s_inodes_per_group;

    uint8_t buffer[BLOCK_SIZE];
    read_inode_bitmap(buffer,group_number);

    if (state == 1)
    {
        bitmap_set(buffer,group_index);
        fext2_groups_table[group_number].bg_free_inodes_count --;
    }    
    else
    {
        bitmap_clear(buffer,group_index);
        fext2_groups_table[group_number].bg_free_inodes_count ++;
    }

    // TODO:这些文件并不支持多线程操作
    // 直接更新位图数据
    device_seek(fext2_groups_table[group_number].bg_inode_bitmap*BLOCK_SIZE);
    device_write(buffer,BLOCK_SIZE);
    device_fflush();
}

/**
 * @brief 
 * 获取inode中某一数据块的内容
 * @param block 存储
 * @param data_block_index 数据块索引 inode 内索引 
 * @param inode 
 * @return Bool 
 */
Bool read_inode_data_block(void * block,uint32_t data_block_index, const struct fext2_inode  *  inode )
{
    
    if (data_block_index < FEXT2_N_BLOCKS-1) 
    {
        return read_data_block(block, inode->i_block[data_block_index]);
    }else // 处于间接块中
    {

        uint32_t tmp_block[BLOCK_SIZE/sizeof(uint32_t)];
        uint32_t offset = data_block_index - FEXT2_N_BLOCKS + 1;
        if (offset >= (BLOCK_SIZE/sizeof(uint32_t)))
        {
            DBG_PRINT("This offset more than limit");
            return FALSE;
        }

        read_data_block(tmp_block, inode->i_block[FEXT2_N_BLOCKS-1]);
        
        // 从指定偏移获取块号 随后读取存储的地址所指向的块号
        return read_data_block(block,tmp_block[offset]);
    }
}

/**
 * @brief 
 * 从索引节点表中读inode数据
 * @param ino 分配的ino号
 * @return struct fext2_inode* 
 */
struct fext2_inode *  read_inode(uint32_t ino)
{


    // 
    uint32_t group_number = (ino-1)/ fext2_sb.s_inodes_per_group;
    if (group_number >= NUM_GROUP) 
        return NULL;
    /*偏移 代表第几个inode*/
    uint32_t offset = (ino-1)%fext2_sb.s_inodes_per_group; 
    if (offset >= fext2_sb.s_inodes_per_group)
        return NULL;
    /*索引表起始地址*/
    uint32_t base = fext2_groups_table[group_number].bg_inode_table;

    // 获得该节点的 需要记得释放
    struct fext2_inode* ret = (struct fext2_inode *)malloc(sizeof(struct fext2_inode));

    /*块内偏移*/
    uint32_t block_num = offset/(BLOCK_SIZE/sizeof(struct fext2_inode));
    uint32_t block_offset = offset%(BLOCK_SIZE/sizeof(struct fext2_inode));
    
    // 读取数据
    device_seek((base+block_num)*BLOCK_SIZE + (block_offset*sizeof(struct fext2_inode)));
    device_read(ret, sizeof(struct fext2_inode));

    return ret;
}

/**
 * @brief 
 * 向磁盘文件同步inode 数据
 * @param inode 含数据的inode
 * @param ino 具体的ino号 从1开始
 * @return uint32_t 
 */
uint32_t write_inode(const struct fext2_inode * inode,uint32_t ino)
{
    
    uint32_t group_number = (ino-1)/fext2_sb.s_inodes_per_group;
    if (group_number >= NUM_GROUP) 
        return 0;
    uint32_t offset =(ino-1)%fext2_sb.s_inodes_per_group;

    uint32_t base = fext2_groups_table[group_number].bg_inode_table;

    uint32_t block_num = offset/(BLOCK_SIZE/sizeof(struct fext2_inode));
    uint32_t block_offset = offset%(BLOCK_SIZE/sizeof(struct fext2_inode));

    device_seek((base+block_num)*BLOCK_SIZE + (block_offset * sizeof(struct fext2_inode)));
    device_write((void *)inode, sizeof(struct fext2_inode));
    device_fflush();
    return ino;
}

/**
 * @brief 
 * 向当前的inode结构新增数据块号
    如果第7块数据块无法存储数据 则会申请一个块
    同时第7块指向 间接块中的0块
    采用遍历的方式检索结构的哪些部分为0
    优先前面的0会被处理 后才会处理
 * @param ino 
 * @param inode 
 * @return 块内索引 
 */
int32_t wirte_ino_for_inode(uint32_t block_no, struct fext2_inode * inode,uint32_t ino)
{
    if (inode->i_blocks == FEXT2_MAX_BLOCKS)
    {
        DBG_PRINT("have not able to add new inode, > max");
        return -1;
    }

    uint32_t cur = 0;
    for (; cur < FEXT2_N_BLOCKS-1; ++cur) 
    {
        if (!inode->i_block[cur])
        {
            inode->i_block[cur] = block_no;
            return cur;
        }
    }
    if (cur == FEXT2_N_BLOCKS-1) 
    {
        
        if (!inode->i_block[FEXT2_N_BLOCKS-1])
        {
            uint32_t group_number = (ino-1)/fext2_sb.s_inodes_per_group;
            uint32_t blockno_in = get_unused_block(group_number);
            if (!blockno_in)
                return -1;
            block_bitmap_set(blockno_in, 1);
            inode->i_block[FEXT2_N_BLOCKS-1] = blockno_in;
            uint32_t new_block[BLOCK_SIZE/sizeof(uint32_t)]={0};
            new_block[0] = block_no;
            write_data_blcok(new_block,blockno_in); // 同步数据到磁盘中
            return FEXT2_N_BLOCKS-1;
        }
        else
        {
            uint32_t block_buffer[BLOCK_SIZE/sizeof(uint32_t)]={0};
            read_data_block(block_buffer, inode->i_block[FEXT2_N_BLOCKS-1]);
            uint32_t remain_num = (inode->i_blocks - FEXT2_N_BLOCKS+1);
            uint32_t i = 0; 
            for (; i < remain_num; ++i)
            {
                if (!block_buffer[i])
                {
                    block_buffer[i] = block_no;
                    write_data_blcok(block_buffer,inode->i_block[FEXT2_N_BLOCKS-1]); 
                    return (FEXT2_N_BLOCKS-1)+i;
                }
            }
            if (i == remain_num) 
            {
                block_buffer[remain_num] = block_no;
                write_data_blcok(block_buffer,inode->i_block[FEXT2_N_BLOCKS-1]); 
                return (FEXT2_N_BLOCKS-1)+remain_num;
            }
        }
    }
    return -1;
}

Bool write_inode_data_block(void * block,uint32_t data_block_index, const  struct fext2_inode * inode)
{

    if (data_block_index < FEXT2_N_BLOCKS-1) 
    {
        return write_data_blcok(block, inode->i_block[data_block_index]);
    }
    else // 处于间接块中
    {

        uint32_t tmp_block[BLOCK_SIZE/sizeof(uint32_t)];
        uint32_t offset = data_block_index - FEXT2_N_BLOCKS + 1;
        if (offset >= (BLOCK_SIZE/sizeof(uint32_t)))
        {
            DBG_PRINT("This offset more than limit");
            return FALSE;
        }
        read_data_block(tmp_block, inode->i_block[FEXT2_N_BLOCKS-1]);
        write_data_blcok(block,tmp_block[offset]);
    }
    return TRUE;
}
/**
 * @brief 
 * 依据某data_block_index释放该块的内容
 * @param data_block_index 
 * @param inode 
 * @return Bool 
 */
Bool free_inode_data_block(uint32_t data_block_index,struct fext2_inode * inode)
{

    if (data_block_index < FEXT2_N_BLOCKS-1) 
    {
        block_bitmap_set(inode->i_block[data_block_index], 0);
        inode->i_block[data_block_index] = 0;
    }
    else 
    {

        uint32_t tmp_block[BLOCK_SIZE/sizeof(uint32_t)];
        uint32_t offset = data_block_index - FEXT2_N_BLOCKS + 1;
        read_data_block(tmp_block, inode->i_block[FEXT2_N_BLOCKS-1]);
        // 获取
        block_bitmap_set(tmp_block[offset], 0);
        tmp_block[offset] = 0;
        write_data_blcok(tmp_block,inode->i_block[FEXT2_N_BLOCKS-1]); // 更新块数据
    }
    return TRUE;
}
