#include <io.h>
#include <command.h>
#include <system.h>
#include <directorystack.h>
#include "header/filesystem/fat32.h"

#define MAX_COMMAND_LENGTH 4000

int main(void) {
    struct DirectoryStack dir_stack; 
    init_dir(&dir_stack);

    char buf[MAX_COMMAND_LENGTH];
    while(true) {
        print_cwd(&dir_stack);
        get_line(buf);
        command(buf, &dir_stack);
        puts("\n");
    }

    return 0;
}
