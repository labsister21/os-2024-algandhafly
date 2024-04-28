#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include "header/filesystem/fat32.h"

#define MAX_DIR_LENGTH 256
#define DIR_NAME_LENGTH 8

extern struct FAT32DriverState user_fat32_state;

void systemCall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

void get_dir_str(char *dir_str);
uint8_t get_dir(struct FAT32DirectoryTable *dir_table, const char folderName[DIR_NAME_LENGTH]);
uint8_t get_cwd(struct FAT32DirectoryTable *dir_table);
bool is_in_root();
void change_directory(char folderName[DIR_NAME_LENGTH]);

uint8_t make_directory(char folderName[DIR_NAME_LENGTH]);

bool is_empty(struct FAT32DirectoryEntry *entry);
bool is_directory(struct FAT32DirectoryEntry *entry);
bool is_file(struct FAT32DirectoryEntry *entry);

uint8_t read_file(struct FAT32DirectoryEntry *entry, char *buf);
uint8_t delete_file_or_dir(struct FAT32DirectoryEntry *entry);

// Codes:
// 0: Read
// 1: Read Directory
// 2: Write
// 3: Delete
// 4: Get Keyboard Buffer
// 5: Kernel Puts
// 6: Kernel Gets
// 7: Keyboard State Activate

#endif
