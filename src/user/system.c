#include <system.h>
#include <string.h>

void systemCall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

struct FAT32DriverState user_fat32_state;
uint32_t current_parent_cluster = 2;

char current_directory[DIR_NAME_LENGTH] = "root";
char current_directory_path[MAX_DIR_LENGTH] = "/";


void init_user_driver_state(){
    current_parent_cluster = 2;
    get_dir(&user_fat32_state.dir_table_buf, "root\0\0\0\0");
}

uint8_t get_cwd(struct FAT32DirectoryTable *dir_table) {
    return get_dir(dir_table, current_directory);
}

bool is_in_root() {
    return memcmp(current_directory, "root", 4) == 0;
}

void change_directory(char folderName[DIR_NAME_LENGTH]){
    memcpy(current_directory, folderName, DIR_NAME_LENGTH);
}
void get_dir_str(char *dir_str) {
    memcpy(dir_str, current_directory, DIR_NAME_LENGTH);
}

uint8_t get_dir(struct FAT32DirectoryTable *dir_table, const char folderName[DIR_NAME_LENGTH]) {
    struct FAT32DriverRequest request;
    request.parent_cluster_number = current_parent_cluster;
    memcpy(request.name, folderName, DIR_NAME_LENGTH);
    request.buf = dir_table;

    uint8_t error_code;
    systemCall(1, (uint32_t )&request, (uint32_t )&error_code, 0);
    return error_code;
}

uint8_t make_directory(char folderName[DIR_NAME_LENGTH]) {
    struct FAT32DriverRequest request = {
        .parent_cluster_number = current_parent_cluster,
        .buffer_size = 0,
    };
    memcpy(request.name, folderName, DIR_NAME_LENGTH);
    memcpy(request.ext, "\0\0\0", 3);

    uint8_t error_code;
    systemCall(2, (uint32_t )&request, (uint32_t )&error_code, 0);
    return error_code;
}

bool is_empty(struct FAT32DirectoryEntry *entry) {
    return entry->user_attribute != UATTR_NOT_EMPTY;
}
bool is_directory(struct FAT32DirectoryEntry *entry) {
    return entry->attribute == ATTR_SUBDIRECTORY;
}
bool is_file(struct FAT32DirectoryEntry *entry) {
    return entry->attribute != ATTR_SUBDIRECTORY;
}

uint8_t read_file(struct FAT32DirectoryEntry *entry, char *buf) {
    struct FAT32DriverRequest request = {
        .parent_cluster_number = current_parent_cluster,
        .buffer_size = entry->filesize,
    };
    memcpy(request.name, entry->name, 8);
    memcpy(request.ext, entry->ext, 3);
    request.buf = buf;

    uint8_t error_code;
    systemCall(0, (uint32_t )&request, (uint32_t )&error_code, 0);

    return error_code;
}

uint8_t delete_file_or_dir(struct FAT32DirectoryEntry *entry) {
    struct FAT32DriverRequest request = {
        .parent_cluster_number = current_parent_cluster,
        .buffer_size = 0,
    };
    memcpy(request.name, entry->name, 8);
    memcpy(request.ext, entry->ext, 3);

    uint8_t error_code;
    systemCall(3, (uint32_t )&request, (uint32_t )&error_code, 0);
    return error_code;
}

