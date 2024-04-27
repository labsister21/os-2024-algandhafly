#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/cpu/portio.h"
// #include <sys/io.h>

uint16_t* framebuffer = (uint16_t*)(FRAMEBUFFER_MEMORY_OFFSET);

void framebuffer_set_cursor(uint8_t r, uint8_t c) {
    uint16_t pos = r * WIDTH + c;

    out(CURSOR_PORT_CMD, 0x0F);
    out(CURSOR_PORT_DATA, (uint8_t)(pos & 0xFF));
    out(CURSOR_PORT_CMD, 0x0E); //
    out(CURSOR_PORT_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg) {
    uint16_t attrib = (bg << 4) | (fg & 0x0F);
    size_t index = row * WIDTH + col;
    framebuffer[index] = c | (attrib << 8);
}

void framebuffer_clear(void) {
    for (size_t y = 0; y < HEIGHT; ++y) {
        for (size_t x = 0; x < WIDTH; ++x) {
            size_t index = y * WIDTH + x;
            framebuffer[index] = ' ' | (0x0F << 8) | (0x00 << 12);
        }
    }
}

void framebuffer_write_length(uint8_t row, uint8_t col, const char* str, size_t length, uint8_t fg, uint8_t bg) {
    if (length == 0) return;
    size_t i = row;
    size_t j = col;
    size_t str_pos = 0;
    while(i < HEIGHT && j < WIDTH && str_pos < length) {
        framebuffer_write(i, j, str[str_pos], fg, bg);
        str_pos++;
        j++;
        if (j == WIDTH) {
            j = 0;
            i++;
        }
    }
}

void framebuffer_write_int(uint8_t row, uint8_t col, int num, uint8_t fg, uint8_t bg) {
    if(num == 0) {
        framebuffer_write(row, col, '0', fg, bg);
        return;
    }
    char str[32];
    size_t i = 0;
    bool is_negative = false;
    if(num < 0) {
        num = -num;
        is_negative = true;
    }
    while(num > 0) {
        str[i] = (char)(num % 10 + '0');
        num /= 10;
        i++;
    }
    
    char fliped_str[i+1];
    if(is_negative) {
        fliped_str[0] = '-';
        i++;
        for(size_t j = 1; j < i; j++) {
            fliped_str[j] = str[i-j-1];
        }
    } else {
        for(size_t j = 0; j < i; j++) {
            fliped_str[j] = str[i-j-1];
        }
    }
    framebuffer_write_length(row, col, fliped_str, i, fg, bg);
}

void framebuffer_write_until_null(uint8_t row, uint8_t col, const char* str, uint8_t fg, uint8_t bg) {
    size_t i = 0;
    while(str[i] != '\0') {
        framebuffer_write(row, col, str[i], fg, bg);
        i++;
        col++;
        if(col == WIDTH) {
            col = 0;
            row++;
        }
    }
}

void put_char(char c, uint32_t color)
{
    framebuffer_write_length(0, 0, &c, 1, 0x0F, 0x00); // sementara
    // TODO: IMPLEMENT
}

void puts(const char *str, uint32_t color)
{
    framebuffer_write_until_null(0, 0, str, 0x0F, 0x00); // sementara
    // TODO: IMPLEMENT
}