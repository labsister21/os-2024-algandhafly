#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define FRAMEBUFFER_MEMORY_OFFSET ((uint8_t*) 0xC00B8000)
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
void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg);

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


void put_char(char c, uint32_t color);

void puts(const char *str, uint32_t count, uint32_t color);

#endif