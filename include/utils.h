

#include "common.h"
#include "device.h"
#include "fuse-ext2/fext2.h"
#include "fuse-ext2/types.h"


Bool read_data_block(void * block,uint32_t block_number);

Bool write_data_blcok(void * block,uint32_t block_number);
