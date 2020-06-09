#include "my_tar_main.h"

#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>


int main( int argc, const char* argv[] )
{
    if (argc < 3)
    {
        my_str_write(1, "Not enough input! Exiting...\n");
        return 1;
    }

    struct options opts = my_getopt(argv);
    if (opts.failure == true)
    {
        my_str_write(1, "Unknown input flag is specified! Stopping tar ...\n");
        return 1;
    }

    int num_files = my_int_max(argc - 3, 0);

    const char *filename = argv[2];
    const char *files[num_files];
    for (size_t i = 3; i < argc; ++i)
    {
        files[i - 3] = argv[i];
    }

    struct my_tar_type **tar = create_tar_double_ptr();    

    if (opts.is_file == true)
    {
        if (opts.is_create == true)
        {
            int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
            my_tar_write(fd, tar, files, num_files);
            close(fd);
        }
        else if (opts.is_append == true)
        {
            const int fd = open(filename, O_RDWR);
            my_tar_read(fd, tar);
            my_tar_write(fd, tar, files, num_files);
            close(fd);
        }
        else if (opts.is_ls == true)
        {
            const int fd = open(filename, O_RDWR);
            my_tar_read(fd, tar);
            my_tar_print(*tar);
            close(fd);
        }
        else if (opts.is_update == true)
        {
            const int fd = open(filename, O_RDWR);
            my_tar_read(fd, tar);
            my_tar_update(fd, tar, files, num_files);
            close(fd);
        }
        else if (opts.is_extract == true)
        {
            const int fd = open(filename, O_RDWR);
            my_tar_read(fd, tar);
            my_tar_extract(fd, *tar, files, num_files);
            close(fd);
        }
        else
        {
            // Do nothing
        }
    }
    else
    {
        my_str_write(1, "No -f is specified! Stopping ...\n");
        return 1;
    }

    free_tar_double_ptr(tar);
    return 0;
}
