#include <io.h>

#define MAX_COMMAND_LENGTH 100

int main(void) {
    puts_color("OSLahPokokNya", Color_LightGreen, Color_Black);
    puts_color(">", Color_LightBlue, Color_Black);
    puts_color("$ ", Color_Yellow, Color_Black);

    set_active_keyboard(true);
    
    char buf[MAX_COMMAND_LENGTH];
    get_line(buf);

    puts_color("\nYou entered: ", Color_LightGreen, Color_Black);
    puts_color(buf, Color_White, Color_Black);

    return 0;
}
