#include <cwdlist.h>
#include <stdint.h>
#include <system.h>
#include <string.h>
#include <io.h>

uint8_t set_dir(struct CWDList* cwd_list, uint8_t index, char next_dir[DIR_NAME_LENGTH]){
    if(index < 0 || index >= cwd_list->length) return 1;
    memcpy(cwd_list->dir[index], next_dir, DIR_NAME_LENGTH);
    cwd_list->length++;
    return 0;
}
uint8_t remove_dir(struct CWDList* cwd_list, uint8_t index){
    if(index < 0 || index >= cwd_list->length) return 1;
    memcpy(cwd_list->dir + index*DIR_NAME_LENGTH, cwd_list->dir + (index+1)*DIR_NAME_LENGTH, cwd_list->length*DIR_NAME_LENGTH - (index+1)*DIR_NAME_LENGTH);
    cwd_list->length--;
    return 0;
}
uint8_t append_dir(struct CWDList* cwd_list, char next_dir[DIR_NAME_LENGTH]){
    memcpy(cwd_list->dir[cwd_list->length], next_dir, DIR_NAME_LENGTH);
    cwd_list->length++;
    return 0;
}
uint8_t remove_last_dir(struct CWDList* cwd_list){
    cwd_list->length--;
    return 0;
}
char* last_dir(struct CWDList* cwd_list){
    return cwd_list->dir[cwd_list->length-1];
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
