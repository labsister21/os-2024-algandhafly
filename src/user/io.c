#include <io.h>
#include <system.h>

void puts(char *str) {
    systemCall(5, (uint32_t) str, Color_White, Color_Black);
}
void puts_color(char *str, uint8_t fg, uint8_t bg) {
    systemCall(5, (uint32_t) str, fg, bg);
}

void put_char(char c) {
    systemCall(5, (uint32_t) &c, 0, 0);
}

void gets(char *buf) {
    systemCall(6, (uint32_t) buf, 0, 0);
}

void get_line(char *buf) {
    systemCall(6, (uint32_t) buf, Color_White, Color_Black);
}
void get_line_color(char *buf, uint8_t fg, uint8_t bg) {
    systemCall(6, (uint32_t) buf, fg, bg);
}

void set_active_keyboard(bool active) {
    systemCall(7, active, 0, 0);
}

void clear_screen() {
    systemCall(8, 0, 0, 0);
}
void set_cursor(uint8_t x, uint8_t y) {
    systemCall(9, x, y, 0);
}

