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

    const char *flags = argv[1];
    int c = 0;
    int r = 0;
    int t = 0;
    int u = 0;
    int x = 0;

    int f = 0;
    int C = 0;

    for(int i = 0; i < my_str_len(flags); ++i)
    {
        if (flags[i] == '-')
        {
            continue;
        }
        else if (flags[i] == 'c')
        {
            c = 1;
        }
        else if (flags[i] == 'r')
        {
            r = 1;
        }
        else if (flags[i] == 't')
        {
            t = 1;
        }
        else if (flags[i] == 'u')
        {
            u = 1;
        }
        else if (flags[i] == 'x')
        {
            x = 1;
        }
        else if (flags[i] == 'f')
        {
            f = 1;
        }
        else
        {
            my_str_write(1, "Unknown input flag is specified! Stopping tar ...\n");
            return 1;
        }
    }   

    int num_files = my_int_max(argc - 3, 0);

    const char *filename = argv[2];
    const char *files[num_files];
    for (size_t i = 3; i < argc; ++i)
    {
        files[i - 3] = argv[i];
    }

    struct my_tar_type **tar = create_tar_double_ptr();    

    if (f == 1)
    {
        if (c == 1)
        {
            int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
            my_tar_write(fd, tar, files, num_files);
            close(fd);
        }
        else if (r == 1)
        {
            const int fd = open(filename, O_RDWR);
            int num_files_read = my_tar_read(fd, tar);
            my_tar_write(fd, tar, files, num_files);
            close(fd);
        }
        else if (t == 1)
        {
            const int fd = open(filename, O_RDWR);
            int num_files_read = my_tar_read(fd, tar);
            my_tar_print(*tar);
            close(fd);
        }
        else if (u == 1)
        {
            const int fd = open(filename, O_RDWR);
           // int num_files_read = my_tar_read(fd, tar);
            my_tar_update(fd, tar, files, num_files);
            close(fd);
        }
        else if (x == 1)
        {
            const int fd = open(filename, O_RDWR);
            int num_files_read = my_tar_read(fd, tar);
            int num_files_extracted = my_tar_extract(fd, *tar);
            close(fd);
        }
        else
        {

        }
    }
    else
    {
        my_str_write(1, "No -f is specified! Stopping ...\n");
    }
    

    free_tar_double_ptr(tar);
    return 0;
}