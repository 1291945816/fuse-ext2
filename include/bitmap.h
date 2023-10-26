#ifndef INCLUDE_BITMAP_H
#define INCLUDE_BITMAP_H

#include "common.h"



int get_zero_bit(uint8_t * buffer);


void bitmap_set(uint8_t * buffer,uint32_t index);
void bitmap_clear(uint8_t * buffer,uint32_t index);



#endif