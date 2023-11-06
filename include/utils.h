

#include "common.h"
#include "device.h"
#include "fuse-ext2/fext2.h"
#include "fuse-ext2/types.h"
#include <stdint.h>

#define IS_DIR(i_mode) \
(((i_mode) & __S_IFDIR)  == __S_IFDIR)  

#define IS_REG(i_mode) \
(((i_mode) & __S_IFREG)  == __S_IFREG)


Bool read_data_block(void * block,uint32_t block_number);

Bool write_data_blcok(void * block,uint32_t block_number);

int find_char(const char * src,char c);
