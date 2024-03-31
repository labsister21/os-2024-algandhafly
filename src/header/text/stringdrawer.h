#ifndef _TXT_DRAWER
#define _TXT_DRAWER

#include "header/text/framebuffer.h"

#define Black 0
#define Blue 1
#define Green 2
#define Cyan 3
#define Red	4
#define Magenta 5
#define Brown 6
#define Light 7
#define DarkGray 8
#define LightBlue 9
#define LightGreen 10
#define LightCyan 11
#define LightRed 12
#define LightMagenta 13
#define Yellow 14
#define White 15

uint16_t char_to_color(char c);

void drawBg(uint16_t bgColor, int y, int x);

void drawBgFromStringList(const char** stringList);

void drawTextWhite(char* string, int y, int x);

void drawWelcomeScreen();

#endif