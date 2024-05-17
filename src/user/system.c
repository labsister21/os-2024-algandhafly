#include <system.h>
#include <string.h>
#include "header/driver/clock.h"

void systemCall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

// populates dir_table
// @return Error code: 0 success - 1 not a folder - 2 not found - -1 unknown
uint8_t get_dir(char folder_name[DIR_NAME_LENGTH], uint16_t parent_cluster_number, struct FAT32DirectoryTable* dir_table) {
    struct FAT32DriverRequest request;

    request.parent_cluster_number = parent_cluster_number;
    memcpy(request.name, folder_name, DIR_NAME_LENGTH);
    request.buf = dir_table;

    uint8_t error_code;
    systemCall(1, (uint32_t )&request, (uint32_t )&error_code, 0);
    return error_code;
}


void get_dir_by_cluster(uint16_t current_cluster_number, struct FAT32DirectoryTable* dir_table) {
    struct FAT32DriverRequest request;
    request.parent_cluster_number = current_cluster_number;
    request.buf = dir_table;
    systemCall(10, (uint32_t )&request, 0, 0);
}

uint8_t get_file_dir(char folder_name[DIR_NAME_LENGTH], char ext[DIR_EXT_LENGTH], uint16_t parent_cluster_number, struct FAT32DirectoryTable *dir_table){
    struct FAT32DriverRequest request;

    request.parent_cluster_number = parent_cluster_number;
    memcpy(request.name, folder_name, DIR_NAME_LENGTH);
    memcpy(request.ext, ext, DIR_EXT_LENGTH);
    request.buf = dir_table;

    uint8_t error_code;
    systemCall(1, (uint32_t )&request, (uint32_t )&error_code, 0);
    return error_code;
}

// write folder to storage
// return Error code: 0 success - 1 file/folder already exist - 2 invalid parent cluster - -1 unknown
uint8_t make_directory(char folder_name[DIR_NAME_LENGTH], uint16_t parent_cluster_number) {
    struct FAT32DriverRequest request = {
        .parent_cluster_number = parent_cluster_number,
        .buffer_size = 0,
    };
    memcpy(request.name, folder_name, DIR_NAME_LENGTH);
    memcpy(request.ext, "\0\0\0", DIR_EXT_LENGTH);
    

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

// populates buf
uint8_t read_file(struct FAT32DirectoryEntry *entry, uint16_t parent_cluster_number, char *buf) {
    struct FAT32DriverRequest request = {
        .parent_cluster_number = parent_cluster_number,
        .buffer_size = entry->filesize,
    };
    memcpy(request.name, entry->name, DIR_NAME_LENGTH);
    memcpy(request.ext, entry->ext, DIR_EXT_LENGTH);
    request.buf = buf;

    uint8_t error_code;
    systemCall(0, (uint32_t )&request, (uint32_t )&error_code, 0);

    return error_code;
}

// delete file or dir in storage
uint8_t delete_file_or_dir(struct FAT32DirectoryEntry *entry, uint16_t parent_cluster) {
    struct FAT32DriverRequest request = {
        .parent_cluster_number = parent_cluster,
        .buffer_size = 0,
    };
    memcpy(request.name, entry->name, DIR_NAME_LENGTH);
    memcpy(request.ext, entry->ext, DIR_EXT_LENGTH);

    uint8_t error_code;
    systemCall(3, (uint32_t )&request, (uint32_t )&error_code, 0);
    return error_code;
}

uint8_t write_file(struct FAT32DirectoryEntry *entry, uint16_t parent_cluster, char* buf){
    struct FAT32DriverRequest request = {
        .parent_cluster_number = parent_cluster,
        .buffer_size = entry->filesize,
        .buf = buf
    };
    memcpy(request.name, entry->name, DIR_NAME_LENGTH);
    memcpy(request.ext, entry->ext, DIR_EXT_LENGTH);

    uint8_t error_code;
    systemCall(2, (uint32_t )&request, (uint32_t )&error_code, 0);
    return error_code;
}


// exec
uint8_t execute_file(struct FAT32DirectoryEntry *entry, uint16_t parent_cluster){
    struct FAT32DriverRequest request = {
        .parent_cluster_number = parent_cluster,
    };
    memcpy(request.name, entry->name, DIR_NAME_LENGTH);
    memcpy(request.ext, entry->ext, DIR_EXT_LENGTH);

    uint8_t error_code;
    uint16_t pid;
    systemCall(11, (uint32_t )&request, (uint32_t )&error_code, &pid);
    
    return error_code;
}

// ps
uint8_t get_process_list(struct FAT32DirectoryEntry *entry, uint16_t parent_cluster){
    struct FAT32DriverRequest request = {
        .parent_cluster_number = parent_cluster,
        .buffer_size = entry->filesize,
    };
    memcpy(request.name, entry->name, DIR_NAME_LENGTH);
    memcpy(request.ext, entry->ext, DIR_EXT_LENGTH);

    uint8_t error_code;
    systemCall(12, (uint32_t )&request, (uint32_t )&error_code, 0);
    return error_code;
}


time_t get_current_time(){
    time_t current_time;
    systemCall(14, (uint32_t )&current_time, 0, 0);
    return current_time;
}

void activate_clock_screen(){
    systemCall(15, 0, 0, 0);
}

void exit_user_shell(){
    systemCall(16, 0, 0, 0);
}
