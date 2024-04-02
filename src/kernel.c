#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/kernel-entrypoint.h"
#include "header/text/framebuffer.h"
#include "header/text/stringdrawer.h"
#include "header/interrupt/interrupt.h"
#include "header/interrupt/idt.h"
#include "header/driver/disk.h"
#include "header/driver/keyboard.h"
#include "header/filesystem/fat32.h"


/**
 * Keyboard havent handle enter key perfectly:
 * - Not handling edge cases
 * 
*/

char frame[WIDTH][HEIGHT];

void handleEnterKey(int *row, int *col) {
    *row += 1;
    *col = 0;
    framebuffer_set_cursor(*row, *col);
}

void kernel_setup(void) {
    
    // === Milestone 0 ===
    // load_gdt(&_gdt_gdtr);

    // === Milestone 1.1 ===
    // framebuffer_clear();
    // drawWelcomeScreen();
    // framebuffer_set_cursor(79, 24);

    // === Milestone 1.2 ===
    // load_gdt(&_gdt_gdtr);
    // pic_remap();
    // initialize_idt();
    // framebuffer_clear();
    // framebuffer_set_cursor(0, 0);
    // __asm__("int $0x4");

    // === Milestone 1.3 ===
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
        
    int col = 0, row = 0;
    keyboard_state_activate();
    while (true) {
         char c;
         get_keyboard_buffer(&c);
         if (c == '\b') {
            if (col == 0 && row == 0) continue;

            if (col == 0 && row > 0) {
                row--;
                col = 79;
            } else {
                col--;
            }

            framebuffer_write(row, col, ' ', 0xF, 0);
            framebuffer_set_cursor(row, col);



         } else if (c == '\n') {
            handleEnterKey(&row, &col);
         }  else if (c) {
            framebuffer_write(row, col++, c, 0xF, 0);
            framebuffer_set_cursor(row, col);
         }

         if (col == 80) {
            col = 0;
            row++;
            framebuffer_set_cursor(row, col);
         }
    }


    // === Milestone 1.4 ===
    // load_gdt(&_gdt_gdtr);
    // pic_remap();
    // activate_keyboard_interrupt();
    // initialize_idt();

    // struct BlockBuffer b;
    // for (int i = 0; i < 512; i++) b.buf[i] = 3;
    // write_blocks(&b, 0, 1);

    // struct BlockBuffer b2;
    // read_blocks(&b2, 0, 1);
    // for (int i = 0 ; i < 128; i++) b2.buf[i] *= 2;
    // write_blocks(&b2, 0, 1);

    while (true);
}