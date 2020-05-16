#include "tar_main.h"

#include <unistd.h>

void my_write(int fd, char *input)
{
    if (input != NULL)
    {
        write(fd, input, my_str_len(input));
    }
    else 
    {
        write(fd, "(null)", 6);    
    }
}

size_t my_str_len(char *input)
{
    size_t count = 0;
    while(input++)
    {
        ++count;
    }
    return count;
}