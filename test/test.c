
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "bitmap.h"
#include "debug.h"
#include"device.h"
#include "utils.h"
#include"fuse-ext2/types.h"
#include"fuse-ext2/fext2.h"
#include "fuse-ext2/fext2_init.h"







int main()
{

    // const char * path = "/Ate/Abc";
    // const uint32_t len = strlen(path);

    // char * parent_path = (char*)malloc(sizeof(char)* len+1);
    // memset(parent_path, '\0', len+1);
    // char dir_name[FEXT2_MAX_NAME_LEN]={0};

    // int ret = parse_cur_dir(path, parent_path, dir_name);

    // printf("%s\n",parent_path);
    // printf("%s\n",dir_name);










    device_open("/home/psong/fuse-ext2/image/fext2");
    // erase_disk();
    read_superblock();
    read_group_desc();

    // // 尝试创建一个A文件 B文件
    // // 首先分配一个inode号
    // // get_unused_inode(1);
    struct fext2_inode a;

    a.i_uid = getuid();
    a.i_gid = getgid();
    a.i_atime = a.i_ctime = a.i_mtime = time(NULL);
    a.i_dtime = 0;
    a.i_block[0] = get_unused_block(0);
    a.i_blocks=1;
    a.i_mode = 0664 |  __S_IFREG; // 文件模式
    a.i_link_count = 1;
    a.i_size = 0;
    block_bitmap_set(a.i_block[0], 1);
    uint32_t ino = get_unused_inode(1);
    inode_bitmap_set(ino, 1);
    struct fext2_dir_entry file_entry;
    strcpy(file_entry.file_name, "D");
    file_entry.name_len = strlen("D");
    file_entry.file_type = REG;  // 这是一个文件
    file_entry.ino = ino;
    file_entry.rec_len = sizeof(struct fext2_dir_entry);

    struct fext2_inode * roo_dir= read_inode(INODE_ROOT_INO);

    Bool ret = add_entry(INODE_ROOT_INO, roo_dir, &file_entry);
    DBG_PRINT("result is %d", ret);
    write_inode(&a,ino);
    struct fext2_inode * r=read_inode(ino);
    DBG_PRINT("%d: %d %d",ino,a.i_blocks,r->i_blocks);
    write_inode(roo_dir, INODE_ROOT_INO); // 也要更新目录信息
    update_group_desc(); // 更新块组信息

    // // // 查询新增的目录/ 文件
    // struct fext2_inode * root = read_inode(INODE_ROOT_INO);
    // // find_entry(root, "A", );
    // uint32_t ino = lookup_inode_by_name(root, "abc");
    // // struct fext2_inode * A = read_inode(ino);
    // DBG_PRINT("%d", ino);

    // free(root);

    // free(parent_path);










    









    device_close();
    return 0;
}