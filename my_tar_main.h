
#include <stddef.h>

struct my_tar_type
{
    /* POSIX header */
    /// @{
    char name[100]; 
    char mode[8];              
    char uid[8];               
    char gid[8];               
    char size[12];             
    char mtime[12];            
    char chksum[8];            
    char typeflag;             
    char linkname[100]; 
    /// @}

    char block[512];

    struct my_tar_type *next;
};

void my_str_write(int fd, const char *str);
size_t my_str_len(const char *str);
int my_int_max(int left, int right);
int my_int_min(int left, int right);