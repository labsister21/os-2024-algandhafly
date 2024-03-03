#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/kernel-entrypoint.h"
#include "header/text/framebuffer.h"

void kernel_setup(void) {
    framebuffer_clear();
    framebuffer_write(3, 12, 'k', 0, 0xF);
    framebuffer_write(3, 13, 'o', 0, 0xF);
    framebuffer_write(3, 14, 'd', 0, 0xF);
    framebuffer_write(3, 15, 'o', 0, 0xF);
    framebuffer_write(3, 16, 'k', 0, 0xF);
    framebuffer_write(3, 17, '!', 0, 0xF);
    framebuffer_set_cursor(3, 18);
    while (true);
}