#include "fuse-ext2/fext2.h"
#include "debug.h"
#include "device.h"
#include <stdlib.h>

void * fext2_init(struct fuse_conn_info *conn)
{
    DBG_PRINT("start initialize fext2 filesystem...\n");
    // 初始化超级块
    read_superblock();
    // 初始化块表
    read_group_desc();

    DBG_PRINT("successfully.\n");

}
void fext2_destory(void * data)
{
    // 同步数据
    update_superblock();
    update_group_desc();

    device_fflush();
    device_close(); // 销毁指针
    free(fext2_groups_table);
    DBG_PRINT("filesystem exited\n");
}