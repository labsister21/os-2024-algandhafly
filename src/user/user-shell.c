#include <io.h>
#include <command.h>
#include <system.h>
#include <directorystack.h>
#include <string.h>
#include "header/filesystem/fat32.h"

#define MAX_COMMAND_LENGTH 4000

int main(void) {
    // path_to_dir_stack("ayam/be/cicak.txt/sa.cpp/buaya.pp", &stack);
    // print_path_to_cwd(&stack); puts("\n");
    // path_to_dir_stack("ayam/be/cicak.txt/sa.cpp/buaya.pp/", &stack);
    // print_path_to_cwd(&stack); puts("\n");
    // path_to_dir_stack("ayam/be/cicak.txt/sa.cpp/buaya", &stack);
    // print_path_to_cwd(&stack); puts("\n");
    // path_to_dir_stack("ayam/be/cicak.txt/sa.cpp/buaya/", &stack);
    // print_path_to_cwd(&stack); puts("\n");


    struct DirectoryStack dir_stack; 
    init_dir(&dir_stack);


    char buf[MAX_COMMAND_LENGTH];
    while(true) {
        print_cwd(&dir_stack);
        get_line(buf);
        command(buf, &dir_stack);
        memset(buf, 0, MAX_COMMAND_LENGTH); // Reset the command buffer
    }
    

    return 0;
}
