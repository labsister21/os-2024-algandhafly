#ifndef _TXT_DRAWER
#define _TXT_DRAWER

#include "header/text/framebuffer.h"


uint16_t char_to_color(char c);

void drawBg(uint16_t bgColor, int y, int x);

void drawBgFromStringList(const char** stringList);

void drawTextWhite(char* string, int y, int x);

void drawWelcomeScreen();

#endif