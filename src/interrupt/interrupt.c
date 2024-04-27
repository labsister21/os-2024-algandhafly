#include "header/interrupt/interrupt.h"
#include "header/cpu/portio.h"
#include "header/text/stringdrawer.h"
#include "header/driver/keyboard.h"
#include "header/cpu/gdt.h"
#include "header/filesystem/fat32.h"
#include "header/text/framebuffer.h"

struct TSSEntry _interrupt_tss_entry = {
    .ss0  = GDT_KERNEL_DATA_SEGMENT_SELECTOR,
};



void io_wait(void) {
    out(0x80, 0);
}

void pic_ack(uint8_t irq) {
    if (irq >= 8) out(PIC2_COMMAND, PIC_ACK);
    out(PIC1_COMMAND, PIC_ACK);
}

void pic_remap(void) {
    // Starts the initialization sequence in cascade mode
    out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); 
    io_wait();
    out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC1_DATA, PIC1_OFFSET); // ICW2: Master PIC vector offset
    io_wait();
    out(PIC2_DATA, PIC2_OFFSET); // ICW2: Slave PIC vector offset
    io_wait();
    out(PIC1_DATA, 0b0100); // ICW3: tell Master PIC, slave PIC at IRQ2 (0000 0100)
    io_wait();
    out(PIC2_DATA, 0b0010); // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    out(PIC1_DATA, ICW4_8086);
    io_wait();
    out(PIC2_DATA, ICW4_8086);
    io_wait();

    // Disable all interrupts
    out(PIC1_DATA, PIC_DISABLE_ALL_MASK);
    out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}


void main_interrupt_handler(struct InterruptFrame frame) {
    switch (frame.int_number) {
        case PIC1_OFFSET + IRQ_KEYBOARD:
            keyboard_isr();
            break;
    }
    
    
}


void activate_keyboard_interrupt(void) {
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_KEYBOARD));
}

void set_tss_kernel_current_stack(void) {
    uint32_t stack_ptr;
    // Reading base stack frame instead esp
    __asm__ volatile ("mov %%ebp, %0": "=r"(stack_ptr) : /* <Empty> */);
    // Add 8 because 4 for ret address and other 4 is for stack_ptr variable
    _interrupt_tss_entry.esp0 = stack_ptr + 8; 
}

void syscall(struct InterruptFrame frame) {
    switch (frame.cpu.general.eax) {
        case 0:
            *((int8_t*) frame.cpu.general.ecx) = read(
                *(struct FAT32DriverRequest*) frame.cpu.general.ebx
            );
            break;
        case 1:
            *((int8_t*) frame.cpu.general.ecx) = read_directory(
                *(struct FAT32DriverRequest*) frame.cpu.general.ebx
            );
            break;
        case 2:
            *((int8_t*) frame.cpu.general.ecx) = write(
                *(struct FAT32DriverRequest*) frame.cpu.general.ebx
            );
            break;
        case 3:
            *((int8_t*) frame.cpu.general.ecx) = delete(
                *(struct FAT32DriverRequest*) frame.cpu.general.ebx
            );
            break;
        case 4:
            get_keyboard_buffer((char*) frame.cpu.general.ebx);
            break;
        case 5:
            char c = frame.cpu.general.eax;
            uint32_t color = frame.cpu.general.ebx;
            put_char(c, color);
            break;
        case 6: 
            char*string = (char*)frame.cpu.general.ebx;
            uint32_t count = frame.cpu.general.ecx;
            uint32_t color_ = frame.cpu.general.edx; 
            puts(string, count, color_);
            break;
        case 7: 
            keyboard_state_activate();
            break;
        case 8:
            uint8_t row, col;
            row = frame.cpu.general.ebx;
            col = frame.cpu.general.ecx;
            framebuffer_set_cursor(row, col);
            break;
        case 9:
            framebuffer_clear();
            break;
        case 10:
            // framebuffer_write_int();
        case 11:
            // framebuffer_write_length();
    }
}
