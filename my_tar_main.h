#include <stddef.h>
#include <unistd.h>

/* POSIX header constants */
#define NAME_MAX_ELEMENT 100                 // 100
#define MODE_MAX_ELEMENT 8                   // 108
#define UID_MAX_ELEMENT 8                    // 116
#define GID_MAX_ELEMENT 8                    // 124
#define SIZE_MAX_ELEMENT 12                  // 136
#define MTIME_MAX_ELEMENT 12                 // 148  
#define CHKSUM_MAX_ELEMENT 8                 // 156  
#define LINKNAME_MAX_ELEMENT 100             // 256      

/* UStar format header constants */
#define SEVERAL_OLD_FIELDS_MAX_ELEMENT 156   // 156
#define ALSO_LINK_NAME_MAX_ELEMENT 100       // 256
#define USTAR_MAX_ELEMENT 8                  // 
#define OWNER_MAX_ELEMENT 32
#define GROUP_MAX_ELEMENT 32
#define MAJOR_MAX_ELEMENT 8
#define MINOR_MAX_ELEMENT 8
#define PREFIX_MAX_ELEMENT 155

// BLOCK size
#define BLOCK_MAX_ELEMENT 512

/* Type flag constants */
const static char NORMAL_FILE = '0';
const static char HARD_LINK = '1';
const static char SYMBOLIC_LINK = '2';
const static char DIRECTORY = '5';

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
    //char link;             
    //char linkname[LINKNAME_MAX_ELEMENT]; 
    /// @}

    /* UStar format */
    /// @{
    //char several_old_fields[SEVERAL_OLD_FIELDS_MAX_ELEMENT];             
    char typeflag;                  
    //char also_link_name[ALSO_LINK_NAME_MAX_ELEMENT];  
    //char ustar[USTAR_MAX_ELEMENT];              
    //char owner[OWNER_MAX_ELEMENT];           
    //char group[GROUP_MAX_ELEMENT];            
    //char major[MAJOR_MAX_ELEMENT];             
    //char minor[MINOR_MAX_ELEMENT];             
    //char prefix[PREFIX_MAX_ELEMENT];
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
void my_full_str_copy(char *dest, int *dest_str_ind, const char *src, size_t src_sz);


void decimal_to_octal(char *dest, int input, int placeholder);
int populate_block(struct my_tar_type *tar);

int get_tar_checksum(struct my_tar_type *tar);