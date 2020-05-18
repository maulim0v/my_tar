#include "my_tar_main.h"

#include <stdlib.h>
#include <unistd.h>

struct my_tar_type* create_tar_ptr()
{
    struct my_tar_type *tar = (struct my_tar_type *)malloc(sizeof(struct my_tar_type));
    init_tar_ptr(tar);
    return tar;
}

void init_tar_ptr(struct my_tar_type *tar)
{
    const char init_char = '\0';
    init_char_array(tar->name, NAME_MAX_ELEMENT, init_char);
    init_char_array(tar->mode, MODE_MAX_ELEMENT, init_char);
    init_char_array(tar->uid, UID_MAX_ELEMENT, init_char);
    init_char_array(tar->gid, GID_MAX_ELEMENT, init_char);
    init_char_array(tar->size, SIZE_MAX_ELEMENT, init_char);
    init_char_array(tar->mtime, MTIME_MAX_ELEMENT, init_char);
    init_char_array(tar->chksum, CHKSUM_MAX_ELEMENT, init_char);
    tar->typeflag = init_char;
    //init_char_array(tar->linkname, LINKNAME_MAX_ELEMENT, init_char);
    init_char_array(tar->block, BLOCK_MAX_ELEMENT, init_char);
}

void init_char_array(char *str, int size, const char init_char)
{
    for(int i = 0; i < size; ++i)
    {
        str[i] = init_char;
    }
}

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

void my_str_copy(char *dest, const char *src)
{
    size_t src_sz = my_str_len(src);
    for(int i = 0; i < src_sz; ++i)
    {
        dest[i] = src[i];
    }
}

void my_full_str_copy(char *dest, int *dest_str_ind, const char *src, size_t src_sz)
{
    int str_ind = *dest_str_ind;
    int i = 0;
    for(; (str_ind + i) < (str_ind + src_sz); ++i)
    {
        dest[str_ind + i] = src[i];
    }
    *dest_str_ind = str_ind + i;
}

void decimal_to_octal(char *dest, int input, int placeholder)
{
    for (int i = 0; i < placeholder; ++i)
    {
        dest[i] = '0';
    }

    int count = 0;
    int sum = 0;
    while (input > 0)
    {
        int rem = input % 8;
        dest[placeholder - 1 - count] = rem + 48;
        input /= 8;
        ++count;
    }
}

int populate_block(struct my_tar_type *tar)
{
    int str_ind = 0;
    my_full_str_copy(tar->block, &str_ind, tar->name, NAME_MAX_ELEMENT);
    my_full_str_copy(tar->block, &str_ind, tar->mode, MODE_MAX_ELEMENT);
    my_full_str_copy(tar->block, &str_ind, tar->uid, UID_MAX_ELEMENT);
    my_full_str_copy(tar->block, &str_ind, tar->gid, GID_MAX_ELEMENT);
    my_full_str_copy(tar->block, &str_ind, tar->size, SIZE_MAX_ELEMENT);
    my_full_str_copy(tar->block, &str_ind, tar->mtime, MTIME_MAX_ELEMENT);
    my_full_str_copy(tar->block, &str_ind, tar->chksum, CHKSUM_MAX_ELEMENT);
    {
        tar->block[str_ind++] = tar->typeflag;
    }

    return str_ind;
}

int get_tar_checksum(struct my_tar_type *tar)
{
    int checksum = 0;
    for(int i = 0; i < BLOCK_MAX_ELEMENT; ++i)
    {
        checksum += (int) tar->block[i];
    }
    return checksum;
}