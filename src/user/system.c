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

void init_user_driver_state(){
    get_dir(&user_fat32_state.dir_table_buf, "root\0\0\0\0");
}

void get_dir(struct FAT32DirectoryTable *dir_table, const char folderName[8]) {
    struct FAT32DriverRequest request;
    request.parent_cluster_number = 2;
    memcpy(request.name, folderName, 8);
    request.buf = dir_table;

    uint8_t error_code;
    systemCall(1, (uint32_t )&request, (uint32_t )&error_code, 0);
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