#ifndef INCLUDE__DEBUG_H
#define INCLUDE__DEBUG_H

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
#endif