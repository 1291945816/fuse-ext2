
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

    read_superblock();
    read_group_desc();

    uint8_t buffer[BLOCK_SIZE];

    read_block_bitmap(buffer, 1);
    int32_t block_index=get_unused_block(1);
    DBG_PRINT("%d", block_index);
    block_bitmap_set(block_index, 1);
    read_block_bitmap(buffer, 1);
    print(buffer,BLOCK_SIZE);



    device_close();
    

    

    
    
    




    return 0;
}