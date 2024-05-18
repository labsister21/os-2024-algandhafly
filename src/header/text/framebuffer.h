#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Before: 0xB8000
#define FRAMEBUFFER_MEMORY_OFFSET ((uint64_t*) 0xC00B8000) 
#define CURSOR_PORT_CMD    0x03D4
#define CURSOR_PORT_DATA   0x03D5
#define WIDTH 80
#define HEIGHT 25

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

extern struct FramebufferState framebuffer_state;
struct FramebufferState {
    uint8_t cursor_x;
    uint8_t cursor_y;
} __attribute__((packed));

/**
 * Terminal framebuffer
 * Resolution: 80x25
 * Starting at FRAMEBUFFER_MEMORY_OFFSET,
 * - Even number memory: Character, 8-bit
 * - Odd number memory:  Character color lower 4-bit, Background color upper 4-bit
*/

/**
 * Set framebuffer character and color with corresponding parameter values.
 * More details: https://en.wikipedia.org/wiki/BIOS_color_attributes
 *
 * @param row Vertical location (index start 0)
 * @param col Horizontal location (index start 0)
 * @param c   Character
 * @param fg  Foreground / Character color
 * @param bg  Background color
 */
void framebuffer_write(int8_t row, int col, char c, uint8_t fg, uint8_t bg);

/**
 * Set cursor to specified location. Row and column starts from 0
 * 
 * @param r row
 * @param c column
*/
void framebuffer_set_cursor(uint8_t r, uint8_t c);

/**
 * Set all cell in framebuffer character to 0x00 (empty character)
 * and color to 0x07 (gray character & black background)
 * Extra note: It's allowed to use different color palette for this
 *
 */
void framebuffer_clear(void);

/**
 * Set framebuffer characters and color until length.
 * @param row Vertical location (index start 0)
 * @param col Horizontal location (index start 0)
 * @param c   Character
 * @param c   Length
 * @param fg  Foreground / Character color
 * @param bg  Background color
 */
void framebuffer_write_length(uint8_t row, uint8_t col, const char* str, size_t length, uint8_t fg, uint8_t bg);

/**
 * Set framebuffer characters and color until length.
 * @param row Vertical location (index start 0)
 * @param col Horizontal location (index start 0)
 * @param num integer
 * @param fg  Foreground / Character color
 * @param bg  Background color
 */
void framebuffer_write_int(uint8_t row, uint8_t col, int num, uint8_t fg, uint8_t bg);

// return the length of the string
void framebuffer_write_and_move_cursor_until_null(const char* str, uint8_t fg, uint8_t bg);


void kernel_puts(char *str, uint8_t fg, uint8_t bg);

void kernel_puts_with_overflow_handling(char *str, uint8_t fg, uint8_t bg);

void kernel_get_line(char *buf, uint8_t fg, uint8_t bg);

void increment(struct FramebufferState *framebuffer_state);

void decrement(struct FramebufferState *framebuffer_state);

void clear_bottom_screen();

#endif