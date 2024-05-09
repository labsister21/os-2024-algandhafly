#include <cwdlist.h>
#include <stdint.h>
#include <system.h>
#include <string.h>
#include <io.h>

uint8_t push_dir(struct CWDList* cwd_list, char next_dir[DIR_NAME_LENGTH], uint16_t cluster_low){
    if(cwd_list->length == MAX_DIR_LENGTH) return 1;
    memcpy(cwd_list->dir[cwd_list->length], next_dir, DIR_NAME_LENGTH);
    cwd_list->parent_cluster[cwd_list->length] = cluster_low;
    cwd_list->length++;
    return 0;
}
uint8_t pop_dir(struct CWDList* cwd_list){
    if(cwd_list->length == 0) return 1;
    cwd_list->length--;
    return 0;
}
char* last_dir(struct CWDList* cwd_list){
    return cwd_list->dir[cwd_list->length-1];
}
uint16_t prev_parent_cluster(struct CWDList* cwd_list){
    if(cwd_list->length == 1)
        return cwd_list->parent_cluster[0];
    return cwd_list->parent_cluster[cwd_list->length-2];
}
uint16_t current_parent_cluster(struct CWDList* cwd_list){
    return cwd_list->parent_cluster[cwd_list->length-1];
}



void print_cwd(struct CWDList* cwd_list) {
    if(cwd_list->length < 1) return;
    puts_color(OS_ROOT_NAME, Color_LightGreen, Color_Black);
    for(uint8_t i = 1; i < cwd_list->length; i++){
        puts_color(cwd_list->dir[i], Color_LightGreen, Color_Black);
        puts_color("/", Color_LightGreen, Color_Black);
    }
    puts_color(">", Color_LightBlue, Color_Black);
    puts_color("$ ", Color_Yellow, Color_Black);
}

void print_path_to_cwd(struct CWDList* cwd_list) {
    for(uint8_t i = 0; i < cwd_list->length; i++){
        puts_clamped(cwd_list->dir[i], 8);
        puts("/");
    }
}
