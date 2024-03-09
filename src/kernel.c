#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/kernel-entrypoint.h"
#include "header/text/framebuffer.h"
#include "header/text/stringdrawer.h"

void kernel_setup(void) {
    // framebuffer_clear();
    // drawWelcomeScreen();
    // framebuffer_set_cursor(79, 24);

    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    __asm__("int $0x4");
    while (true);
}