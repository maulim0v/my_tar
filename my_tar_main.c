#include "my_tar_main.h"

#include <unistd.h>

void my_str_write(int fd, const char *str)
{
    if (str != NULL)
    {
        write(fd, str, my_str_len(str));
    }
    else 
    {
        write(fd, "(null)", 6);    
    }
}

size_t my_str_len(const char *str)
{
    size_t count = 0;
    while(*str++ != '\0')
    {
        ++count;
    }
    return count;
}

int my_int_max(int left, int right)
{
    return left > right ? left : right; 
}

int my_int_min(int left, int right)
{
    return left < right ? left : right;
}