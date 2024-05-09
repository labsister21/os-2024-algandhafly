#ifndef CWDLIST_H
#define CWDLIST_H

#include <stdint.h>
#include <system.h>

struct CWDList {
    char dir[MAX_DIR_LENGTH][DIR_NAME_LENGTH];
    uint8_t length;
} __attribute__((packed));
uint8_t set_dir(struct CWDList* cwd_list, uint8_t index, char next_dir[DIR_NAME_LENGTH]);
uint8_t remove_dir(struct CWDList* cwd_list, uint8_t index);
uint8_t append_dir(struct CWDList* cwd_list, char next_dir[DIR_NAME_LENGTH]);
uint8_t remove_last_dir(struct CWDList* cwd_list);

// Bassically return only the name of cwd folder
char* last_dir(struct CWDList* cwd_list);
void print_cwd(struct CWDList* cwd_list);

#endif