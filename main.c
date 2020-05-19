
#include "my_tar_main.h"

#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

int main( int argc, const char* argv[] )
{
    if (argc < 3)
    {
        my_str_write(1, "Not enough input! Exiting...\n");
        return 1;
    }

    int num_files = my_int_max(argc - 3, 0);

    const char *filename = argv[2];
    const char *files[num_files];
    for (size_t i = 3; i < argc; ++i)
    {
        files[i - 3] = argv[i];
    }

    const char *test_file = files[0];

    const int fd = open(filename, O_RDONLY);

    struct my_tar_type *tar = create_tar_ptr();

    // struct stat my_stat;
    // int success = stat(test_file, &my_stat);

    // my_str_copy(tar->name, test_file);
    // decimal_to_octal(tar->mode, my_stat.st_mode & 0777, 7);
    // decimal_to_octal(tar->uid, my_stat.st_uid, 7);
    // decimal_to_octal(tar->gid, my_stat.st_gid, 7);
    // decimal_to_octal(tar->size, (int)my_stat.st_size, 11);
    // decimal_to_octal(tar->mtime, (int)my_stat.st_mtime, 11);

    // if ((my_stat.st_mode & S_IFMT) == S_IFREG)
    // {
    //     tar->typeflag = NORMAL_FILE;
    // }
    // else if ((my_stat.st_mode & S_IFMT) == S_IFDIR)
    // {
    //     init_char_array(tar->size, SIZE_MAX_ELEMENT - 1, '0');
    //     tar->typeflag = DIRECTORY;
    // }
    
    // int str_ind = populate_block(tar) - 9;
    // decimal_to_octal(tar->chksum, get_tar_checksum(tar), 6);
    // tar->chksum[6] = '\0';
    // tar->chksum[7] = ' ';
    // my_full_str_copy(tar->block, &str_ind, tar->chksum, CHKSUM_MAX_ELEMENT);

    // int tr = write(fd, tar->block, 512);

    my_tar_read(fd, tar);

    //write(1, tar->block, 512);
    //printf("\n%s\n", tar->name);
    //printf("%s\n", tar->mode);
    //printf("%s\n", tar->uid);
    //printf("%s\n", tar->gid);
    //printf("%s\n", tar->size);
    //printf("%s\n", tar->mtime);
    //printf("%s\n", tar->chksum);
    //rintf("%c\n", tar->typeflag);

    close(fd);

    free(tar);
    return 0;
}