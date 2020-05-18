
#include "my_tar_main.h"

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


    struct my_tar_type *tar = create_tar_ptr();

    struct stat my_stat;
    int success = stat(test_file, &my_stat);

    my_str_copy(tar->name, test_file);
    decimal_to_octal(tar->mode, my_stat.st_mode & 0777, 7);
    decimal_to_octal(tar->uid, my_stat.st_uid, 7);
    decimal_to_octal(tar->gid, my_stat.st_gid, 7);
    decimal_to_octal(tar->size, (int)my_stat.st_size, 11);
    decimal_to_octal(tar->mtime, (int)my_stat.st_mtime, 11);

    populate_block(tar);
    decimal_to_octal(tar->chksum, get_tar_checksum(tar), 6);
    tar->chksum[6] = '\0';

    free(tar);
    return 0;
}