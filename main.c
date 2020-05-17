
#include "my_tar_main.h"

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



    return 0;
}