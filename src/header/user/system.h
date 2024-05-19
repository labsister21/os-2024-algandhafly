#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "header/driver/clock.h"

#define MAX_DIR_LENGTH 256
#define DIR_NAME_LENGTH 8
#define DIR_EXT_LENGTH 3
#define OS_ROOT_NAME "OSLahPokoknya:/"

extern struct FAT32DriverState user_fat32_state;


void systemCall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

uint8_t get_dir(char folder_name[DIR_NAME_LENGTH], uint16_t parent_cluster_number, struct FAT32DirectoryTable *dir_table);
void get_dir_by_cluster(uint16_t current_cluster_number, struct FAT32DirectoryTable* dir_table);
uint8_t get_file_dir(char folder_name[DIR_NAME_LENGTH], char ext[DIR_EXT_LENGTH], uint16_t parent_cluster_number, struct FAT32DirectoryTable *dir_table);



uint8_t make_directory(char folder_name[DIR_NAME_LENGTH], uint16_t parent_cluster_number);

bool is_empty(struct FAT32DirectoryEntry *entry);
bool is_directory(struct FAT32DirectoryEntry *entry);
bool is_file(struct FAT32DirectoryEntry *entry);

uint8_t read_file(struct FAT32DirectoryEntry *entry, uint16_t parent_cluster, char *buf);
uint8_t delete_file_or_dir(struct FAT32DirectoryEntry *entry, uint16_t parent_cluster);
uint8_t write_file(struct FAT32DirectoryEntry *entry, uint16_t parent_cluster, char* buf);

uint8_t move_file_or_folder(struct FAT32DirectoryEntry *entry_source, struct FAT32DirectoryEntry *entry_dest, uint16_t source_parent_cluster, uint16_t target_parent_cluster);

// Codes:
// 0: Read
// 1: Read Directory
// 2: Write
// 3: Delete
// 4: Get Keyboard Buffer
// 5: Kernel Puts
// 6: Kernel Gets
// 7: Keyboard State Activate


// exec
uint8_t execute_file(struct FAT32DirectoryEntry *entry, uint16_t parent_cluster_number);

// ps
uint8_t get_process_list(struct FAT32DirectoryEntry *entry, uint16_t parent_cluster_number);

// kill
uint8_t kill_process(struct FAT32DirectoryEntry *entry, uint16_t parent_cluster_number);

// clock
time_t get_current_time();


#endif
