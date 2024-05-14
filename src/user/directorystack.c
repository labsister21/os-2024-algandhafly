#include <directorystack.h>
#include <stdint.h>
#include <system.h>
#include <string.h>
#include <io.h>

uint8_t push_dir(struct DirectoryStack* dir_stack, struct FAT32DirectoryEntry* entry){
    if(dir_stack->length == MAX_DIR_LENGTH) return 1;
    memcpy(&dir_stack->entry[dir_stack->length], entry, sizeof(struct FAT32DirectoryEntry));
    dir_stack->length++;
    return 0;
}
uint8_t pop_dir(struct DirectoryStack* dir_stack){
    if(dir_stack->length == 0) return 1;
    dir_stack->length--;
    return 0;
}
struct FAT32DirectoryEntry* peek_top(struct DirectoryStack* dir_stack){
    if(dir_stack->length == 0) return 0;
    return &dir_stack->entry[dir_stack->length-1];
}
struct FAT32DirectoryEntry* peek_second_top(struct DirectoryStack* dir_stack){
    if(dir_stack->length < 2) return 0;
    return &dir_stack->entry[dir_stack->length-2];
}

char* last_dir(struct DirectoryStack* dir_stack){
    return dir_stack->entry[dir_stack->length-1].name;
}
uint16_t prev_parent_cluster(struct DirectoryStack* dir_stack){
    if(dir_stack->length == 1)
        return dir_stack->entry[0].cluster_low;
    return dir_stack->entry[dir_stack->length-2].cluster_low;
}
uint16_t current_parent_cluster(struct DirectoryStack* dir_stack){
    return dir_stack->entry[dir_stack->length-1].cluster_low;
}



void print_cwd(struct DirectoryStack* dir_stack) {
    if(dir_stack->length < 1) return;
    puts_color(OS_ROOT_NAME, Color_LightGreen, Color_Black);
    for(uint8_t i = 1; i < dir_stack->length; i++){
        puts_color(dir_stack->entry[i].name, Color_LightGreen, Color_Black);
        puts_color("/", Color_LightGreen, Color_Black);
    }
    puts_color(">", Color_LightBlue, Color_Black);
    puts_color("$ ", Color_Yellow, Color_Black);
}

void print_path_to_cwd(struct DirectoryStack* dir_stack) {
    for(uint8_t i = 0; i < dir_stack->length; i++){
        puts_clamped(dir_stack->entry[i].name, DIR_NAME_LENGTH);
        if(is_directory(&dir_stack->entry[i])) puts("/");
        else {
            puts(".");
            puts_clamped(dir_stack->entry[i-1].ext, DIR_EXT_LENGTH);
        }
    }
}

void print_path_to_cwd_reversed(struct DirectoryStack* dir_stack) {
    for(uint8_t i = dir_stack->length-1; i > 0; i--){
        puts_clamped(dir_stack->entry[i-1].name, DIR_NAME_LENGTH);
        if(is_directory(&dir_stack->entry[i])) puts("/");
        else {
            puts(".");
            puts_clamped(dir_stack->entry[i-1].ext, DIR_EXT_LENGTH);
        }
    }
}


void init_dir(struct DirectoryStack* dir_stack){
    struct FAT32DirectoryEntry root_entry = {
        .name = "root\0\0\0\0",
        .ext = "\0\0\0",
        .cluster_low = 2
    };
    dir_stack->length = 0;
    push_dir(dir_stack, &root_entry);
}