#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/cpu/portio.h"
#include "header/driver/keyboard.h"
#include "header/interrupt/idt.h"

uint16_t* framebuffer = (uint16_t*)(FRAMEBUFFER_MEMORY_OFFSET);
struct FramebufferState framebuffer_state;

int round_down_division(int dividend, int divisor) {
    if (dividend < 0) {
        return (dividend - (divisor - 1)) / divisor;
    } else {
        return dividend / divisor;
    }
}

void framebuffer_set_cursor(uint8_t r, uint8_t c) {
    uint16_t pos = r * WIDTH + c;

    out(CURSOR_PORT_CMD, 0x0F);
    out(CURSOR_PORT_DATA, (uint8_t)(pos & 0xFF));
    out(CURSOR_PORT_CMD, 0x0E); //
    out(CURSOR_PORT_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

void framebuffer_write(int8_t row, int col, char c, uint8_t fg, uint8_t bg) {
    // Handles newline
    row += round_down_division(col, 80);
    while (col < 0) {
        col += 80;
    }
    col %= 80;
    
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

void framebuffer_write_and_move_cursor_until_null(const char* str, uint8_t fg, uint8_t bg) {
    size_t i = 0;
    while(str[i] != '\0') {
        if(str[i] == '\n') {
            framebuffer_state.cursor_x = 0;
            framebuffer_state.cursor_y++;
            i++;
            continue;
        }
        framebuffer_write(framebuffer_state.cursor_y, framebuffer_state.cursor_x, str[i], fg, bg);
        i++;
        framebuffer_state.cursor_x++;
        if(framebuffer_state.cursor_x == WIDTH) {
            framebuffer_state.cursor_x = 0;
            framebuffer_state.cursor_y++;
        }
    }
    framebuffer_set_cursor(framebuffer_state.cursor_y, framebuffer_state.cursor_x);
}


void kernel_puts(char *str, uint8_t fg, uint8_t bg)
{
    framebuffer_write_and_move_cursor_until_null(str, fg, bg); // sementara
    framebuffer_set_cursor(framebuffer_state.cursor_y, framebuffer_state.cursor_x);
}

void kernel_put_char(char c, uint8_t fg, uint8_t bg){
    if(c == '\n'){
        framebuffer_state.cursor_x = 0;
        framebuffer_state.cursor_y++;
        framebuffer_set_cursor(framebuffer_state.cursor_y, framebuffer_state.cursor_x);
        return;
    }
    framebuffer_write(framebuffer_state.cursor_y, framebuffer_state.cursor_x, c, fg, bg);
    increment(&framebuffer_state);
    framebuffer_set_cursor(framebuffer_state.cursor_y, framebuffer_state.cursor_x);
}

void kernel_puts_with_overflow_handling(char *str, uint8_t fg, uint8_t bg)
{
    size_t i = 0;
    while(str[i] != '\0') {
        if(framebuffer_state.cursor_y == HEIGHT-2) {
            kernel_puts("\nPress enter to continue...", Yellow, Black);
            char buf[2000]; buf[0] = '\0';
            get_command_buffer(buf);
            framebuffer_clear();
            framebuffer_state.cursor_x = 0;
            framebuffer_state.cursor_y = 0;
        }
        
        if(str[i] == '\n') {
            framebuffer_state.cursor_x = 0;
            framebuffer_state.cursor_y++;
            i++;
            continue;
        }
        framebuffer_write(framebuffer_state.cursor_y, framebuffer_state.cursor_x, str[i], fg, bg);
        i++;
        framebuffer_state.cursor_x++;
        if(framebuffer_state.cursor_x == WIDTH) {
            framebuffer_state.cursor_x = 0;
            framebuffer_state.cursor_y++;
        }
        
    }
    framebuffer_set_cursor(framebuffer_state.cursor_y, framebuffer_state.cursor_x);
}

void kernel_get_line(char *buf, uint8_t fg, uint8_t bg) {
    __asm__ volatile("sti");
    
    // initialize_idt(); // for some reason, the keyboard interrupt is not working without calling this again
    keyboard_state_activate();

    uint16_t i = 0;
    while (true) {
        char c;
        get_keyboard_buffer(&c);

        if (c == '\b') {
            if(i == 0) continue;
            i--;
            if (framebuffer_state.cursor_x == 0 && framebuffer_state.cursor_y == 0) continue;

            if (framebuffer_state.cursor_x == 0 && framebuffer_state.cursor_y > 0) {
                framebuffer_state.cursor_y--;
                framebuffer_state.cursor_x = 79;
            } else {
                framebuffer_state.cursor_x--;
            }

            framebuffer_write(framebuffer_state.cursor_y, framebuffer_state.cursor_x, ' ', fg, bg);
            framebuffer_set_cursor(framebuffer_state.cursor_y, framebuffer_state.cursor_x);



        } else if (c == '\n') {
            buf[i] = '\0';
            break;
        }  else if (c) {
            framebuffer_write(framebuffer_state.cursor_y, framebuffer_state.cursor_x++, c, fg, bg);
            framebuffer_set_cursor(framebuffer_state.cursor_y, framebuffer_state.cursor_x);
            buf[i] = c;
            i++;
        }

        if (framebuffer_state.cursor_x == 80) {
            framebuffer_state.cursor_x = 0;
            framebuffer_state.cursor_y++;
            framebuffer_set_cursor(framebuffer_state.cursor_y, framebuffer_state.cursor_x);
        }

    } 

    keyboard_state_deactivate();

}

void increment(struct FramebufferState *framebuffer_state) {
    framebuffer_state->cursor_x++;
    if (framebuffer_state->cursor_x == WIDTH) {
        framebuffer_state->cursor_x = 0;
        framebuffer_state->cursor_y++;
    }
}

void decrement(struct FramebufferState *framebuffer_state) {
    if (framebuffer_state->cursor_x == 0 && framebuffer_state->cursor_y == 0) return;

    if (framebuffer_state->cursor_x == 0 && framebuffer_state->cursor_y > 0) {
        framebuffer_state->cursor_y--;
        framebuffer_state->cursor_x = 79;
    } else {
        framebuffer_state->cursor_x--;
    }
}


void clear_bottom_screen() {
    for(uint8_t j = 0; j < 80; j++) {
        framebuffer_write(23, j, ' ', White, Black);
        framebuffer_write(24, j, ' ', White, Black);
    }
}