#include "tar_main.h"

void my_write(int fd, char *input);


size_t my_str_len(char *input)
{
    size_t count = 0;
    while(input++)
    {
        ++count;
    }
    return count;
}