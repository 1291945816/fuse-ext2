#include "fuse-ext2/fext2.h"
#include "debug.h"

void * fext2_init(struct fuse_conn_info *conn)
{
    DBG_PRINT("start init fext2 filesystem...\n");
    // 初始化超级块
    read_superblock();
    // 初始化块表
    read_group_desc();

    DBG_PRINT("successfully.\n");

}