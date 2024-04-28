#include <io.h>
#include <command.h>
#include <system.h>

#define MAX_COMMAND_LENGTH 100

void print_cwd() {
    puts_color("OSLahPokokNya/", Color_LightGreen, Color_Black);
    char cwd[8];
    // get_dir_str(cwd);
    // puts(cwd);
    puts_color(">", Color_LightBlue, Color_Black);
    puts_color("$ ", Color_Yellow, Color_Black);
}

int main(void) {
    init_user_driver_state();
    char buf[MAX_COMMAND_LENGTH];
    struct FAT32DirectoryTable *dir_table;

    while(true) {
        print_cwd();
        get_line(buf);
        command(buf);
        puts("\n");
    }


    return 0;
}
