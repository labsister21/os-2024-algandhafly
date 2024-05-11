#ifndef DIRECTORY_STACK_H
#define DIRECTORY_STACK_H

#include <stdint.h>
#include <system.h>
#include "header/filesystem/fat32.h"

struct DirectoryStack {
    struct FAT32DirectoryEntry entry[MAX_DIR_LENGTH];
    uint8_t length;
} __attribute__((packed));
uint8_t push_dir(struct DirectoryStack* dir_stack, struct FAT32DirectoryEntry* entry);
uint8_t pop_dir(struct DirectoryStack* dir_stack);
struct FAT32DirectoryEntry* peek_top(struct DirectoryStack* dir_stack);
struct FAT32DirectoryEntry* peek_second_top(struct DirectoryStack* dir_stack);

// Bassically return only the name of cwd folder
char* last_dir(struct DirectoryStack* dir_stack);
uint16_t prev_parent_cluster(struct DirectoryStack* dir_stack);
uint16_t current_parent_cluster(struct DirectoryStack* dir_stack);

void print_cwd(struct DirectoryStack* dir_stack);
void print_path_to_cwd(struct DirectoryStack* dir_stack);
void print_path_to_cwd_reversed(struct DirectoryStack* dir_stack);
void init_dir(struct DirectoryStack* dir_stack);

#endif