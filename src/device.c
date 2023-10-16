#include"device.h"
#include "debug.h"

Bool device_open(const char * file_path)
{
    // DBG_PRINT("file_path: %s",file_path);
    fp = fopen(file_path, "r+");
    return (fp != NULL ? TRUE:FALSE);
}


Bool device_close()
{
    fflush(fp);
    fclose(fp);
    return TRUE;
}


Bool device_seek(uint32_t offset)
{
    fseek(fp, offset,SEEK_SET);

    // 返回文件移动的当前指针
    long cur_pos = ftell(fp);
    return cur_pos == offset;

}


Bool device_write(void * buffer,uint32_t size)
{
    size_t write_size=fwrite(buffer,1,size,fp);
    return (write_size == size);
}

Bool device_read(void * buffer,uint32_t size)
{
    size_t  read_count = fread(buffer,1,size,fp);
    return read_count == size;
}



void device_fflush()
{
    fflush(fp);
}