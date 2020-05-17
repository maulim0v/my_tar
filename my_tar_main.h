
#include <stddef.h>

#define NAME_MAX_ELEMENT 100
#define MODE_MAX_ELEMENT 8
#define UID_MAX_ELEMENT 8
#define GID_MAX_ELEMENT 8
#define SIZE_MAX_ELEMENT 12
#define MTIME_MAX_ELEMENT 12
#define CHKSUM_MAX_ELEMENT 8
#define LINKNAME_MAX_ELEMENT 100
#define BLOCK_MAX_ELEMENT 512

struct my_tar_type
{
    /* POSIX header */
    /// @{
    char name[NAME_MAX_ELEMENT]; 
    char mode[MODE_MAX_ELEMENT];              
    char uid[UID_MAX_ELEMENT];               
    char gid[GID_MAX_ELEMENT];               
    char size[SIZE_MAX_ELEMENT];             
    char mtime[MTIME_MAX_ELEMENT];            
    char chksum[CHKSUM_MAX_ELEMENT];            
    char typeflag;             
    char linkname[LINKNAME_MAX_ELEMENT]; 
    /// @}

    char block[BLOCK_MAX_ELEMENT];

    struct my_tar_type *next;
};

struct my_tar_type* create_tar_ptr();
void init_tar_ptr(struct my_tar_type *tar);
void init_char_array(char *str, int size, const char init_char);

void my_str_write(int fd, const char *str);
size_t my_str_len(const char *str);
int my_int_max(int left, int right);
int my_int_min(int left, int right);

void my_str_copy(char *dest, const char *src);


void decimal_to_octal(char *dest, int input, int placeholder);