#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include "header/filesystem/fat32.h"

extern struct FAT32DriverState user_fat32_state;

void systemCall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

void get_dir(struct FAT32DirectoryTable *dir_table, const char folderName[8]);

void make_directory(const char folderName[8]);

bool is_empty(struct FAT32DirectoryEntry *entry);
bool is_directory(struct FAT32DirectoryEntry *entry);
bool is_file(struct FAT32DirectoryEntry *entry);

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
