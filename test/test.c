
#include "bitmap.h"
#include "debug.h"
#include"device.h"
#include"fuse-ext2/types.h"
#include"fuse-ext2/fext2.h"

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>





int main()
{


    device_open("/home/psong/fuse-ext2/image/fext2");

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

    // uint8_t buffer[BLOCK_SIZE];

    // read_block_bitmap(buffer, 0);
    // block_bitmap_set(9, 0);
    // read_block_bitmap(buffer, 0);
    // print(buffer,BLOCK_SIZE);

    // 构建一个inode
    struct fext2_inode inode;

    fext2_groups_table[0].



    // uint32_t block_data[256];
    // block_data[0]=3;         // 表示数据块中的第三个块 

    


    


    device_close();
    

    

    
    
    




    return 0;
}