#include <io.h>
#include <command.h>

#define MAX_COMMAND_LENGTH 100

int main(void) {
    init_user_driver_state();
    char buf[MAX_COMMAND_LENGTH];

    while(true) {
        puts_color("OSLahPokokNya", Color_LightGreen, Color_Black);
        puts_color(">", Color_LightBlue, Color_Black);
        puts_color("$ ", Color_Yellow, Color_Black);

        get_line(buf);
        command(buf);
    }


    return 0;
}
