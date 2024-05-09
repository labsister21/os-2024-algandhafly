#ifndef CWDLIST_H
#define CWDLIST_H

#include <stdint.h>
#include <system.h>

struct CWDList {
    char dir[MAX_DIR_LENGTH][DIR_NAME_LENGTH];
    uint8_t parent_cluster[MAX_DIR_LENGTH];
    uint8_t length;
} __attribute__((packed));
uint8_t push_dir(struct CWDList* cwd_list, char next_dir[DIR_NAME_LENGTH], uint16_t cluster_low);
uint8_t pop_dir(struct CWDList* cwd_list);

// Bassically return only the name of cwd folder
char* last_dir(struct CWDList* cwd_list);
uint16_t prev_parent_cluster(struct CWDList* cwd_list);
uint16_t current_parent_cluster(struct CWDList* cwd_list);

void print_cwd(struct CWDList* cwd_list);

#endif