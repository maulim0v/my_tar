#include "my_tar_main.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

struct my_tar_type* create_tar_ptr()
{
    struct my_tar_type *tar = (struct my_tar_type *)malloc(sizeof(struct my_tar_type));
    init_tar_ptr(tar);
    return tar;
}

void free_tar_ptr(struct my_tar_type *tar)
{
    free(tar);
}

struct my_tar_type** create_tar_double_ptr()
{
    struct my_tar_type **ret = (struct my_tar_type **) malloc (sizeof(struct my_tar_type *));
    return ret;
}

void free_tar_double_ptr(struct my_tar_type **tar)
{
    if (tar == NULL)
    {
        return;
    }

    struct my_tar_type *tmp;
    while(*tar != NULL)
    {
        tmp = *tar;
        *tar = (*tar)->next;
        free_tar_ptr(tmp);
    }

    free(tar);
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

int my_tar_read(int fd, struct my_tar_type **tar)
{
    long unsigned offset = 0;
    int files_read = 0;

    struct my_tar_type **local_tar = tar;

    while(true)
    {
        *local_tar = create_tar_ptr();
        int read_sz = my_file_read(fd, (*local_tar)->block, 512);
        if (read_sz != 512)
        {
            my_str_write(1, "Failed to read block! Stopping reading tar...\n");
            free_tar_ptr(*local_tar);
            *local_tar = NULL;
            break;
        }

        if(is_block_all_zeros((*local_tar)->block, 512) == true)
        {
            read_sz = my_file_read(fd, (*local_tar)->block, 512);
            if (read_sz != 512)
            {
                my_str_write(1, "Failed to read block after 1st all zeros block! Stopping reading tar...\n");
                free_tar_ptr(*local_tar);
                *local_tar = NULL;
                break;
            }

            if(is_block_all_zeros((*local_tar)->block, 512) == true)
            {
                my_str_write(1, "Success on end of tar. Reading is terminating...\n");
                free_tar_ptr(*local_tar);
                *local_tar = NULL;
                break;
            }
        }

        unpopulate_block((*local_tar));    
        (*local_tar)->data_begin_ind = offset;

        int size_to_skip = octal_to_decimal((*local_tar)->size);
        const int rem_to_skip = size_to_skip % 512;
        if (rem_to_skip != 0)
        {
            size_to_skip += 512 - rem_to_skip;
        }

        // Skip block + file size (if any)
        offset += 512 + size_to_skip;
        lseek(fd, size_to_skip, SEEK_CUR);

        local_tar = &(*local_tar)->next;

        ++files_read;
    }
    return files_read;
}

int my_file_read(int fd, char *src, int src_sz)
{
    int read_sz = my_int_max(read(fd, src, src_sz), 0);
    return read_sz;
}

void unpopulate_block(struct my_tar_type *tar)
{
    int str_ind = 0;
    my_full_str_uncopy(tar->name,   NAME_MAX_ELEMENT,   tar->block, &str_ind);
    my_full_str_uncopy(tar->mode,   MODE_MAX_ELEMENT,   tar->block, &str_ind);
    my_full_str_uncopy(tar->uid,    UID_MAX_ELEMENT,    tar->block, &str_ind);
    my_full_str_uncopy(tar->gid,    GID_MAX_ELEMENT,    tar->block, &str_ind);
    my_full_str_uncopy(tar->size,   SIZE_MAX_ELEMENT,   tar->block, &str_ind);
    my_full_str_uncopy(tar->mtime,  MTIME_MAX_ELEMENT,  tar->block, &str_ind);
    my_full_str_uncopy(tar->chksum, CHKSUM_MAX_ELEMENT, tar->block, &str_ind);
    {
        tar->typeflag = tar->block[str_ind++];
    }
}

void my_full_str_uncopy(char* dest, size_t dest_sz, const char *src, int *src_str_ind)
{
    int str_ind = *src_str_ind;
    int i = 0;
    for(; (str_ind + i) < (str_ind + dest_sz); ++i)
    {
        dest[i] = src[str_ind + i];
    }
    *src_str_ind = str_ind + i;
}

int octal_to_decimal(char *src)
{
    int decimal_sum = 0;
    const size_t src_sz = my_str_len(src);    
    for(int i = src_sz - 1; i >= 0; --i)
    {
        const int digit = src[src_sz - 1 - i] - 48;
        if (digit == 0) continue;
        int j = 0;
        int power_8 = 1;
        while(j++ < i)
        {
            power_8 *= 8;
        }
        decimal_sum += digit * power_8;
    }

    return decimal_sum;
}

bool is_block_all_zeros(char *src, int src_sz)
{
    for (int i = 0; i < src_sz; ++i)
    {
        if(src[i] != '\0')
        {
            return false;
        }
    }
    return true;
}

int my_tar_extract(int fd, struct my_tar_type *tar)
{
    // Set file reading to the beginning for tar file
    lseek(fd, 0, SEEK_SET);

    int files_extracted = 0;
    while(tar != NULL)
    {
        // Do something
        my_file_extract(fd, tar);

        tar = tar->next;
        ++files_extracted;
    }

    return files_extracted;
}

int my_file_extract(int fd, struct my_tar_type *tar)
{
    if (tar->typeflag == DIRECTORY)
    {
        int mode_dir = octal_to_decimal(tar->mode) & 0777;
        my_mkdir(tar->name, mode_dir);
    }
    if (tar->typeflag == NORMAL_FILE)
    {
        const int file_sz = octal_to_decimal(tar->size);
        const int file_mode = octal_to_decimal(tar->mode) & 0777;

        const int file_fd = open(tar->name, O_WRONLY | O_CREAT | O_TRUNC, file_mode);
        const int seek_move_to_read = 512 + tar->data_begin_ind;

        // Move current offset (seek) for reading from original tar file
        lseek(fd, seek_move_to_read, SEEK_SET);

        char buff_read[512];
        int sz_read_and_write = 0;

        // To handle when size is way larger than 512 bytes
        while(true)
        {
            int sz_read = file_sz - sz_read_and_write;
            if (sz_read > 512)
            {
                sz_read = 512;
            }
            else if (sz_read <= 0)
            {
                break;
            }

            const int sz_read_ret = my_file_read(fd, buff_read, sz_read);
            if (sz_read_ret < 0)
            {
                my_str_write(1, "Reading from tar failed! Breaking extracting ...\n");
                break;
            }

            const int sz_written_ret = write(file_fd, buff_read, sz_read_ret);
            if (sz_written_ret != sz_read_ret)
            {
                my_str_write(1, "Reading size not equals to writing sz to file! Breaking extracting ...\n");
                break;
            }

            sz_read_and_write += sz_written_ret;
        }

        close(file_fd);
    }

    return 0;
}

int my_mkdir(char *dir, int mode)
{
    const size_t path_len = my_str_len(dir);
    if (path_len < 1)
    {
        return 0;
    }

    char *path = (char *) malloc ( (path_len + 1) * sizeof(char));
    for(int i = 0; i < path_len; ++i)
    {
        if (i == path_len - 1)
        {
            if (dir[i] == '/')
            {
                path[i] = '\0';
            }
        }
        else 
        { 
            path[i] = dir[i];
        }

        if (i == path_len - 1)
        {
            int mkdir_int = mkdir(path, mode);
            //printf("Dir name %s, Mkdir int %d\n", path, mkdir_int);
        }
        else if (path[i] == '/')
        {
            path[i] = '\0';

            int mkdir_int = mkdir(path, mode);
            //printf("Dir name %s, Mkdir int %d\n", path, mkdir_int);
            path[i] = '/';
        }
    }
    path[path_len] = '\0';

    free(path);
    return 0;
}

int my_tar_print(struct my_tar_type *tar)
{
    int files_printed = 0;
    while(tar != NULL)
    {
        my_file_print(tar);

        tar = tar->next;
        ++files_printed;
    }

    return files_printed;
}

int my_file_print(struct my_tar_type *tar)
{
    my_str_write(1, tar->name);
    my_str_write(1, "\n");
    return 0;
}

int my_tar_write(int fd, struct my_tar_type **tar, const char *files[], int num_files)
{
    long unsigned offset = 0;

    my_file_write(fd, tar, files, num_files, &offset);

    my_tar_write_end(fd, offset);

    return offset;
}

int my_file_write(int fd, struct my_tar_type **tar, const char *files[], int num_files, long unsigned *offset_ptr)
{
    struct my_tar_type **local_tar = tar;

    for (int i = 0; i < num_files; ++i)
    {
        *local_tar = create_tar_ptr();
        int format_success = my_file_format(*local_tar, files[i]);
        if (format_success < 0)
        {
            break;
        }

        (*local_tar)->data_begin_ind = *offset_ptr;

        if ((*local_tar)->typeflag == NORMAL_FILE)
        {
            // First write block header metadata into tar file
            const int sz_written_ret = write(fd, (*local_tar)->block, 512);
            if (sz_written_ret != 512)
            {
                my_str_write(1, "Tar writing size not equals to block size 512! Breaking tar writing ...\n");
                break;
            }

            // Next write each file contents into tar file
            char buff_read[512];
            int sz_read_and_write = 0;
            int file_sz = octal_to_decimal((*local_tar)->size);
            
            int file_desc = open((*local_tar) -> name, O_RDONLY);

            // To handle when size is way larger than 512 bytes
            while(true)
            {
                int sz_read = file_sz - sz_read_and_write;
                if (sz_read > 512)
                {
                    sz_read = 512;
                }
                else if (sz_read <= 0)
                {
                    break;
                }

                const int sz_read_ret = my_file_read(file_desc, buff_read, sz_read);
                if (sz_read_ret < 0)
                {
                    my_str_write(1, "Reading from file failed! Breaking tar writing ...\n");
                    my_str_write(1, (*local_tar)->name);
                    my_str_write(1, " file failed! Breaking tar writing ...\n");
                    return -1;
                }

                const int sz_written_ret = write(fd, buff_read, sz_read_ret);
                if (sz_written_ret != sz_read_ret)
                {
                    my_str_write(1, "Reading size not equals to writing sz to file! Breaking tar writing for ");
                    my_str_write(1, (*local_tar)->name);
                    my_str_write(1, " file ...\n");
                    return -1;
                }

                sz_read_and_write += sz_written_ret;
            }

            close(file_desc);

            // Fill the remaining spot with all zeros
            int size_to_fill_with_zeros = file_sz;
            const int rem = size_to_fill_with_zeros % 512;
            if (rem != 0)
            {
                size_to_fill_with_zeros = 512 - rem;
                for (int i = 0; i < size_to_fill_with_zeros; ++i)
                {
                    write(fd, "\0", 1);
                }
                *offset_ptr += size_to_fill_with_zeros;
            }

            // Update offset properly (block + file size)
            *offset_ptr += (file_sz + 512);
        }
        else if ((*local_tar)->typeflag == DIRECTORY)
        {
            
        }

        (*local_tar)->next = NULL;
        local_tar = &(*local_tar)->next;
    }
    return 0;
}

int my_file_format(struct my_tar_type *tar, const char *file_name)
{
    int file_description = open(file_name, O_RDONLY);
    if (file_description < 0)
    {
        my_str_write(1, "Error opening ");
        my_str_write(1, file_name);
        my_str_write(1, " file, Returning from my_file_format!...\n");
        return -1;
    }

    struct stat file_st;
    int success_stat = fstat(file_description, &file_st);
    if (success_stat < 0)
    {
        my_str_write(1, "Failed to stat ");
        my_str_write(1, file_name);
        my_str_write(1, " file, Returning from my_file_format!...\n");
        return -1;
    }

    my_str_copy(tar->name, file_name);
    decimal_to_octal(tar->mode, file_st.st_mode & 0777, 7);
    decimal_to_octal(tar->uid, file_st.st_uid, 7);
    decimal_to_octal(tar->gid, file_st.st_gid, 7);
    decimal_to_octal(tar->size, (int)file_st.st_size, 11);
    decimal_to_octal(tar->mtime, (int)file_st.st_mtime, 11);

    if (S_ISREG(file_st.st_mode) == true)
    {
        tar->typeflag = NORMAL_FILE;
    }
    else if (S_ISDIR(file_st.st_mode) == true)
    {
        init_char_array(tar->size, SIZE_MAX_ELEMENT - 1, '0');
        tar->typeflag = DIRECTORY;
    }
    
    // -9 is offset backwards for checksum
    int str_ind = populate_block(tar) - 9;
    decimal_to_octal(tar->chksum, get_tar_checksum(tar), 6);
    tar->chksum[6] = '\0';
    tar->chksum[7] = ' ';
    my_full_str_copy(tar->block, &str_ind, tar->chksum, CHKSUM_MAX_ELEMENT);

    close(file_description);
    return 0;
}

int my_tar_write_end(int fd, int offset)
{
    for (int i = 0; i < 2 * BLOCK_MAX_ELEMENT; ++i)
    {
        write(fd, "\0", 1);
    }

    return 0;
}
