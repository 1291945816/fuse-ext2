
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>

#include "bitmap.h"
#include "debug.h"
#include"device.h"
#include "utils.h"
#include"fuse-ext2/types.h"
#include"fuse-ext2/fext2.h"
#include "fuse-ext2/fext2_init.h"







int main()
{


    device_open("/home/psong/fuse-ext2/image/fext2");
    read_superblock();
    read_group_desc();


    // 查找 /A/B
    // 先获取根目录的数据
    struct fext2_inode * root_dir = read_inode(INODE_ROOT_INO);
    char * path = "/ABC/"; // ABC/
    uint32_t result = lookup_inode_by_name(root_dir,path+1);
    DBG_PRINT("%d",result);



    // struct fext2_inode *root =  read_inode(2);
    // DBG_PRINT("%d", fext2_groups_table[0].bg_free_inodes_count);
    // uint8_t buffer[BLOCK_SIZE];
    // read_block_bitmap(buffer, 0);
    // print(buffer,BLOCK_SIZE);






    // Bool state = device_seek(1024);

    // printf("%d\n",state);

    // device_close();
    
    // printf("%lu", sizeof(struct fext2_inode));
    // erase_disk();
    // init_meta_info();
    // read_superblock();
    // device_close();
    // read_group_desc();

    // read_superblock();
    // read_group_desc();


    /*测试读取inode块*/
    // 假设 ino 为 3

    // struct fext2_inode inode;
    // inode.i_size = 110;
    // inode.i_blocks = 3;
    // inode.i_atime = time(NULL);

    // // 移动指针
    // device_seek((fext2_groups_table[0].bg_inode_table*BLOCK_SIZE)
    //                                     +2*sizeof(struct fext2_inode));
    // // 写入指定位置
    // device_write(&inode, sizeof(struct fext2_inode));

    // inode_bitmap_set(3, 1);

    // device_fflush();



    // struct fext2_inode* inode_ = read_inode(3);







    // uint8_t buffer[BLOCK_SIZE];

    // read_inode_bitmap(buffer, 0);

    /*获取空的块号*/
    // uint32_t block_no = get_unused_block(0);
    // 构建一个inode
    // struct fext2_inode inode;
    // uint32_t block_data[256];
    // block_data[0]=block_no; //7
    // print(buffer, BLOCK_SIZE);


    // DBG_PRINT("%u", fext2_groups_table[0].bg_free_blocks_count);




    // // 先刷 存储数据块的数据到数据区中 再刷中间块的内容到数据区中
    // char name[BLOCK_SIZE]={"Hello world"};
    // write_data_blcok(name, block_no); // 刷新到文件中
    // block_bitmap_set(block_no, 1); // 状态置为1

    // block_no = get_unused_block(0); 
    // inode.i_block[7] = 2;
    // write_data_blcok(block_data, block_no); // 刷新到文件中
    // block_bitmap_set(block_no, 1); // 状态置为1
    // update_group_desc(); // 更新组描述符


    // 更新间接块
    

    // // 读取数据
    // char name_[BLOCK_SIZE]={0};
    


    // read_inode_data_block(name_, 7, &inode);

    // DBG_PRINT("result is: %s", name_);



    









    // read_block_bitmap(buffer, 0);
    // print(buffer,BLOCK_SIZE);







    // block_data[0]=3;         // 表示数据块中的第三个块 

    

    // DBG_PRINT("%d",(16>>4)<<4);
    
    // free(fext2_groups_table);

    device_close();




    

    

    
    
    




    return 0;
}