#include "my_tar_main.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


struct my_tar_type* create_tar_ptr()
{
    struct my_tar_type *tar = (struct my_tar_type *)malloc(sizeof(struct my_tar_type));
    tar->next = NULL;
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
    *ret = NULL;
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
    init_char_array(tar->linkname, LINKNAME_MAX_ELEMENT, init_char);
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
    my_full_str_copy(tar->block, &str_ind, tar->linkname, LINKNAME_MAX_ELEMENT);

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
    int read_sz = read(fd, src, src_sz);
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
    my_full_str_uncopy(tar->linkname, LINKNAME_MAX_ELEMENT, tar->block, &str_ind);
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

int my_tar_extract(int fd, struct my_tar_type *tar, const char *files[], int num_files)
{
    int files_extracted = 0;

    bool is_C_option_came_up_already = false;
    char *C_option_directory = NULL;
    char *C_path = NULL;

    if (num_files > 0)
    {
        while (tar != NULL)
        {
            for (int i = 0; i < num_files; ++i)
            {
                if (my_str_len(files[i]) == 2 && files[i][0] == '-' && files[i][1] == 'C')
                {
                    if (i < num_files - 1)
                    {
                        is_C_option_came_up_already = true;
                        C_option_directory = (char *)files[i+1];
                        ++i;
                        continue;
                    }
                    else
                    {
                        my_str_write(1, "-C option is specified without directory! Stopping tar writing ...\n");
                        break;
                    }
                }

                if (is_C_option_came_up_already == true)
                {
                    int file_len = my_str_len(files[i]);
                    int C_dir_len = my_str_len(C_option_directory);
                    if (C_option_directory[C_dir_len - 1] == '/')
                    {
                        C_option_directory[C_dir_len - 1] = '\0';
                        --C_dir_len;
                    }
                    C_path = (char *) malloc ( (C_dir_len + 1 + file_len + 1) * sizeof(char) );
                    C_path[0] = '\0';
                    my_str_copy_new(C_path, C_option_directory);
                    my_str_copy_new(C_path, "/");
                    my_str_copy_new(C_path, files[i]);
                }
                else
                {
                    C_path = (char *) malloc ( (my_str_len(files[i]) + 1) * sizeof(char) );
                    C_path[0] = '\0';
                    my_str_copy_new(C_path, files[i]);
                }

                char C_path_arr[my_str_len(C_path) + 1];
                C_path_arr[0] = '\0';
                my_str_copy_new(C_path_arr, C_path);

                if (my_str_compare(tar->name, files[i]) == 1)
                {
                    lseek(fd, tar->data_begin_ind, SEEK_SET);
                    my_file_extract(fd, tar, C_path_arr);
                    ++files_extracted;
                }
                free(C_path);
            }
            is_C_option_came_up_already = false;
            C_option_directory = NULL;
            tar = tar->next;
        }
    }
    else
    {
        // Set file reading to the beginning for tar file
        lseek(fd, 0, SEEK_SET);

        while(tar != NULL)
        {
            // Do something
            my_file_extract(fd, tar, tar->name);

            tar = tar->next;
            ++files_extracted;
        }
    }

    return files_extracted;
}

int my_file_extract(int fd, struct my_tar_type *tar, char *file_name_modified)
{
    if (tar->typeflag == DIRECTORY)
    {
        int mode_dir = octal_to_decimal(tar->mode) & 0777;
        my_mkdir(file_name_modified, mode_dir);
    }
    else if (tar->typeflag == NORMAL_FILE)
    {
        char *path_to_mkdir = (char *) malloc ((my_str_len(file_name_modified) + 2) * sizeof(char));
        path_to_mkdir[0] = '\0';
        my_str_copy_new(path_to_mkdir, "/");
        my_str_copy_new(path_to_mkdir, file_name_modified);
        for(int i = my_str_len(file_name_modified) - 1; i >= 0; --i)
        {
            if (path_to_mkdir[i] == '/')
            {
                path_to_mkdir[i] = '\0';
                break;
            }
        }

        int dir_mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH; // 0755
        my_mkdir(path_to_mkdir, dir_mode);
        
        free(path_to_mkdir);

        const int file_sz = octal_to_decimal(tar->size);
        const int file_mode = octal_to_decimal(tar->mode) & 0777;

        const int file_fd = open(file_name_modified, O_WRONLY | O_CREAT | O_TRUNC, file_mode);
        
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
    else if (tar->typeflag == SYMBOLIC_LINK)
    {
        if (symlink(tar->linkname, tar->name) < 0)
        {
            my_str_write(1, "Unable to make symlink. Breaking ...\n");
        }
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
    path[path_len] = '\0';
    for(int i = 0; i < path_len; ++i)
    {
        if (i == path_len - 1)
        {
            if (dir[i] == '/')
            {
                path[i] = '\0';
            }
            else 
            { 
                path[i] = dir[i];
            }
        }
        else 
        { 
            path[i] = dir[i];
        }

        if (i == path_len - 1)
        {
            int mkdir_int = mkdir(path, mode);
        }
        else if (path[i] == '/')
        {
            path[i] = '\0';

            int mkdir_int = mkdir(path, mode);
            path[i] = '/';
        }
    }

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

    // After tar_reading we need to adjust things around
    if (*tar != NULL)
    {
        // Go to the last element
        while(*tar != NULL && (*tar)->next != NULL)
        {
            tar = &(*tar)->next;
        }

        int size_to_skip = octal_to_decimal((*tar)->size);
        const int rem_to_skip = size_to_skip % 512;
        if (rem_to_skip != 0)
        {
            size_to_skip += 512 - rem_to_skip;
        }

        // Skip everything up until the past last element
        offset += (*tar)->data_begin_ind + size_to_skip + 512;
        lseek(fd, offset, SEEK_SET);

        tar = &(*tar)->next;
    }

    my_file_write(fd, tar, files, num_files, &offset);

    my_tar_write_end(fd, offset);

    return offset;
}

int my_file_write(int fd, struct my_tar_type **tar, const char *files[], int num_files, long unsigned *offset_ptr)
{
    struct my_tar_type **local_tar = tar;

    bool is_C_option_came_up_already = false;
    char *C_option_directory = NULL;
    char *C_path = NULL;

    for (int i = 0; i < num_files; ++i)
    {
        if (my_str_len(files[i]) == 2 && files[i][0] == '-' && files[i][1] == 'C')
        {
            if (i < num_files - 1)
            {
                is_C_option_came_up_already = true;
                C_option_directory = (char *)files[i+1];
                ++i;
                continue;
            }
            else
            {
                my_str_write(1, "-C option is specified without directory! Stopping tar writing ...\n");
                break;
            }
        }

        if (is_C_option_came_up_already == true)
        {
            int file_len = my_str_len(files[i]);
            int C_dir_len = my_str_len(C_option_directory);
            if (C_option_directory[C_dir_len - 1] == '/')
            {
                C_option_directory[C_dir_len - 1] = '\0';
                --C_dir_len;
            }
            C_path = (char *) malloc ( (C_dir_len + 1 + file_len + 1) * sizeof(char) );
            C_path[0] = '\0';
            my_str_copy_new(C_path, C_option_directory);
            my_str_copy_new(C_path, "/");
            my_str_copy_new(C_path, files[i]);            
        }
        else
        {
            C_path = (char *) malloc ( (my_str_len(files[i]) + 1) * sizeof(char) );
            C_path[0] = '\0';
            my_str_copy_new(C_path, files[i]);
        }

        char C_path_arr[my_str_len(C_path) + 1];
        C_path_arr[0] = '\0';
        my_str_copy_new(C_path_arr, C_path);

        *local_tar = create_tar_ptr();
        int format_success = my_file_format(*local_tar, files[i], is_C_option_came_up_already, C_path_arr);
        if (format_success < 0)
        {
            free(C_path);
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
                free(C_path);
                break;
            }

            // Next write each file contents into tar file
            char buff_read[512];
            int sz_read_and_write = 0;
            int file_sz = octal_to_decimal((*local_tar)->size);
            
            int file_desc = open(C_path_arr, O_RDONLY);
            if (file_desc < 0)
            {
                my_str_write(1, "Failed to open/create file for tar writing. Breaking ...\n");
                return -1;
            }
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
                    my_str_write(1, C_path_arr);
                    my_str_write(1, " file failed! Breaking tar writing ...\n");
                    free(C_path);
                    return -1;
                }

                const int sz_written_ret = write(fd, buff_read, sz_read_ret);
                if (sz_written_ret != sz_read_ret)
                {
                    my_str_write(1, "Reading size not equals to writing sz to file! Breaking tar writing for ");
                    my_str_write(1, (*local_tar)->name);
                    my_str_write(1, " file ...\n");
                    free(C_path);
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
           int len_dirname = my_str_len((*local_tar) -> name);
           char * parent = calloc(len_dirname + 1, sizeof(char));
           my_str_copy(parent, (*local_tar) -> name);

            // add a '/' character to the end
            if ((len_dirname < 99) && ((*local_tar) -> name[len_dirname - 1] != '/')){
                (*local_tar) -> name[len_dirname] = '/';
                (*local_tar) -> name[len_dirname + 1] = '\0';
                // get_tar_checksum(*tar);
                decimal_to_octal((*local_tar)->chksum, get_tar_checksum(*local_tar), 6);
                (*local_tar)->chksum[6] = '\0';
                (*local_tar)->chksum[7] = ' ';
            }

            const int sz_written_ret = write(fd, (*local_tar)->block, 512);
            if (sz_written_ret != 512)
            {
                my_str_write(1, "Tar writing size not equals to block size 512! Breaking tar writing ...\n");
                free(C_path);
                break;
            }
            *offset_ptr += 512;


            // directory
            DIR * d = opendir(parent);
            if (!d){
                my_str_write(1, "Cannot open directory, Stopping writing...\n");
                free(parent);
                closedir(d);
                free(C_path);
                break;
            }

            struct dirent * dir;
            while ((dir = readdir(d)))
            {
                const size_t sublen = strlen(dir -> d_name);
                if ((my_str_compare(dir->d_name, ".") != 1 ) && (my_str_compare(dir->d_name, "..") != 1 ))
                {
                    char *path_and_file_name = malloc ((len_dirname+my_str_len(dir->d_name) + 2)* sizeof(char));
                    sprintf(path_and_file_name, "%s/%s", parent, dir -> d_name);
                    if (my_file_write(fd, &((*local_tar) -> next),(const char **) &path_and_file_name, 1, offset_ptr) < 0)
                    {
                        my_str_write(1,"Recurse error\n");
                    }
                                        
                    while( (*local_tar)->next != NULL )
                    {
                        local_tar = &(*local_tar)->next;
                    }
                    free(path_and_file_name);
                }
            }
            free(parent);
            closedir(d);

            *offset_ptr += 512;

        }
        else if ((*local_tar)->typeflag == SYMBOLIC_LINK)
        {
            // Write block header metadata into tar file
            const int sz_written_ret = write(fd, (*local_tar)->block, 512);
            if (sz_written_ret != 512)
            {
                my_str_write(1, "Tar writing size not equals to block size 512! Breaking tar writing ...\n");
                free(C_path);
                break;
            }

            // Update offset properly (block + file size)
            *offset_ptr += (octal_to_decimal((*local_tar)->size) + 512);
        }

        (*local_tar)->next = NULL;
        local_tar = &(*local_tar)->next;
        free(C_path);
    }
    return 0;
}

int my_file_format(struct my_tar_type *tar, const char *file_name, bool is_C_option, char *C_path)
{
    int file_description = -1;
    if (is_C_option == true)
    {
        file_description = open(C_path, O_RDONLY);
    }
    else
    {
        file_description = open(file_name, O_RDONLY);
    }
    
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
    else if (S_ISLNK(file_st.st_mode) == true)
    {
        tar->typeflag = SYMBOLIC_LINK;

        // Get link name
        if (readlink(tar->name, tar->linkname, LINKNAME_MAX_ELEMENT) < 0)
        {
            my_str_write(1, "Could not read link!\n");
        }
    }
    
    // -109 is offset backwards for checksum
    int str_ind = populate_block(tar) - 109;
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

int my_str_compare(const char *left, const char *right)
{
    int left_sz = my_str_len(left);
    int right_sz = my_str_len(right);

    if (left_sz != right_sz)
    {
        return 0;
    }

    for (int i = 0; i < left_sz; ++i)
    {
        if (left[i] != right[i])
        {
            return 0;
        }
    }
    return 1;
}

void my_str_copy_new(char *dest, const char *src)
{
    int dest_sz = my_str_len(dest);
    int src_sz = my_str_len(src);

    for (int i = 0; i < src_sz; ++i)
    {
        dest[dest_sz + i] = src[i];
    }
    dest[dest_sz + src_sz] = '\0';
}


void my_tar_update(int fd, struct my_tar_type **tar, const char *files[], int num_files)
{
    // char ** buffer_for_update = (char **) malloc((num_files+1)*sizeof(char *));
    struct stat st;
    int num_new = 0;
    struct my_tar_type **local_tarr = tar;
    struct my_tar_type *local_tarR;
    const char *new_files[2];
    for(int i = 0; i < num_files; i++)
    {
        my_str_write(1, "Checking file name for tar update: ");
        my_str_write(1, files[i]);

        // Make sure original file exists
        if (lstat(files[i], &st))
        {
            my_str_write(1,"Problem with stat of this file");
        }   
        if (find(*local_tarr, files[i])) 
        {  
            local_tarR =find(*local_tarr, files[i]);
            
            if (st.st_mtime >octal_to_decimal(find(*local_tarr, files[i])->mtime) ) 
            {
                new_files[0] = local_tarR->name; 
                my_tar_write(fd, tar, new_files, num_files);

            } else 
            {
                my_str_write(1, "Everything is up to date. No need to write\n");
            }
            } else 
            {
            my_str_write(1, "This is new file and should be added!\n");
            new_files[0] = files[i]; 
            my_tar_write(fd, tar, new_files, num_files);
        }
    }
}

struct my_tar_type * find(struct my_tar_type * tar, const char * filename){

    while (tar)
    {
        if (my_str_compare(tar -> name, filename)==1) {
            return tar;
        }
        
       tar = tar -> next;
    }
    return NULL;
}