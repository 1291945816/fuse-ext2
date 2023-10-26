#ifndef INCLUDE__DEBUG_H
#define INCLUDE__DEBUG_H
#include "common.h"
/**
 * 方便调试输出
*/
// #include<stdio.h>

#ifdef FEXT2_DEBUG

#define DBG_PRINT(fmt, args...)    \
        do{ \
            printf("====%s:%s:%.4d====: "fmt"\n\r",__FILE__,__FUNCTION__,__LINE__,##args); \
        }while (0)
#else
#define  DBG_PRINT(fmt, args...)

#endif


/**
 * @brief 用以打印位图数据 便于测试
 * 
 * @param buffer 位图数据
 * @param num 大小
 */
static inline void print(uint8_t *buffer,uint32_t num)
{
    for (int i= 0; i < num ; i++) 
    {
        uint8_t tmp = buffer[i];
        for (int j = 0; j < 8; j++) {

            printf("%d ",(tmp & 1));
            tmp = tmp >> 1;
        }
        printf("\n");
    }
}
#endif