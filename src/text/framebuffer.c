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
    volatile uint16_t * where;
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
