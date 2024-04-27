#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>
#include <stdbool.h>

#define Color_Black 0
#define Color_Blue 1
#define Color_Green 2
#define Color_Cyan 3
#define Color_Red	4
#define Color_Magenta 5
#define Color_Brown 6
#define Color_Light 7
#define Color_DarkGray 8
#define Color_LightBlue 9
#define Color_LightGreen 10
#define Color_LightCyan 11
#define Color_LightRed 12
#define Color_LightMagenta 13
#define Color_Yellow 14
#define Color_White 15

void puts(char *str);
void puts_color(char *str, uint8_t fg, uint8_t bg);

void put_char(char c);

void gets(char *buf);

void get_line(char *buf);

void get_line_color(char *buf, uint8_t fg, uint8_t bg);

void set_active_keyboard(bool active);

void clear_screen();

void set_cursor(uint8_t x, uint8_t y);

#endif
