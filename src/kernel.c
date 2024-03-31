#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/kernel-entrypoint.h"
#include "header/text/framebuffer.h"
#include "header/text/stringdrawer.h"
#include "header/interrupt/interrupt.h"
#include "header/interrupt/idt.h"
#include "header/driver/disk.h"
#include "header/filesystem/fat32.h"


void kernel_setup(void) {
    
    // Milestone 0
    // load_gdt(&_gdt_gdtr);

    // Milestone 1.1
    // framebuffer_clear();
    // drawWelcomeScreen();
    // framebuffer_set_cursor(79, 24);

    // Milestone 1.2
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    __asm__("int $0x4");





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