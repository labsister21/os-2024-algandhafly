#include "header/interrupt/interrupt.h"
#include "header/cpu/portio.h"
#include "header/text/stringdrawer.h"
#include "header/driver/keyboard.h"
#include "header/cpu/gdt.h"
#include "header/filesystem/fat32.h"
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/driver/clock.h"
#include "header/scheduler/scheduler.h"

int str_to_int(char* str) {
    int num = 0; 
    for (int i = 0; str[i] != '\0'; i++) { 
        num = num * 10 + (str[i] - 48); 
    } 
    return num; 
}


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
    struct FAT32DriverRequest *request = (struct FAT32DriverRequest*)frame.cpu.general.ebx;
    switch (frame.cpu.general.eax) {
        case 0:
            *((int8_t*) frame.cpu.general.ecx) = read(
                *request
            );
            break;
        case 1:
            *((int8_t*) frame.cpu.general.ecx) = read_directory(
                *(struct FAT32DriverRequest*)frame.cpu.general.ebx
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
            get_keyboard_buffer((char*)frame.cpu.general.ebx);
            break;
        case 5:
            kernel_puts_with_overflow_handling((char*)frame.cpu.general.ebx, frame.cpu.general.ecx, frame.cpu.general.edx);
            break;
        case 6: 
            keyboard_state.show_on_screen = true;
            get_command_buffer((char*)frame.cpu.general.ebx);
            keyboard_state.show_on_screen = false;
            break;
        case 7: 
            activate_keyboard_interrupt();
            keyboard_state_activate();
            break;
        case 8:
            framebuffer_clear();
            break;
        case 9:
            framebuffer_set_cursor(frame.cpu.general.ebx, frame.cpu.general.ecx);
            framebuffer_state.cursor_x = frame.cpu.general.ecx;
            framebuffer_state.cursor_y = frame.cpu.general.ebx;
            break;
        case 10: // had to resort to this because directory[0] == directory[1] == parent cluster. the problem is you can only jump 2 levels up or 2 levels down. cant do 1
            // get sibling directory
            read_clusters(request->buf, request->parent_cluster_number, 1);
            break;
        case 11: // exec

            break;
        case 12: // ps
            kernel_puts("\n", 15, 0);
            for (int i = 0; i < PROCESS_COUNT_MAX; i++) {
                if (!_process_list[i].metadata.state == RUNNING) continue;
                char x[2];
                x[0] = '0' + i;
                x[1] = '\0';
                kernel_puts(x, 15, 0);
                kernel_puts(" ", 15, 0);
                kernel_puts(_process_list[i].metadata.name, 15, 0); //process name
                
                if (_process_list[i].metadata.ext[0] == '\0') continue;
                kernel_puts(".", 15, 0);
                kernel_puts(_process_list[i].metadata.ext, 15, 0);
            }
            break;
        case 13: // kill
            int pid = str_to_int(frame.cpu.general.ecx);
            process_omae_wa_mou_shindeiru(pid);
            break;
        case 14: // get current time
                get_indonesian_time((time_t*)frame.cpu.general.ebx);
            break;
        case 15: // activate clock
                activate_clock();
            break;
        case 16: // stop user shell child process
            break;
    }
}

void main_interrupt_handler(struct InterruptFrame frame) {
    switch (frame.int_number) {
        case PIC1_OFFSET + IRQ_KEYBOARD:
            keyboard_isr();
            pic_ack(IRQ_KEYBOARD);
            break;
        case PIC1_OFFSET + IRQ_TIMER:

            activate_clock();
            scheduler_switch_to_next_process(frame);
            break;
        case 0x30:
            syscall(frame);
            break;
    }
}



// Timer interrupt
#define PIT_MAX_FREQUENCY   1193182
#define PIT_TIMER_FREQUENCY 1
#define PIT_TIMER_COUNTER   (PIT_MAX_FREQUENCY / PIT_TIMER_FREQUENCY)

#define PIT_COMMAND_REGISTER_PIO          0x43
#define PIT_COMMAND_VALUE_BINARY_MODE     0b0
#define PIT_COMMAND_VALUE_OPR_SQUARE_WAVE (0b011 << 1)
#define PIT_COMMAND_VALUE_ACC_LOHIBYTE    (0b11  << 4)
#define PIT_COMMAND_VALUE_CHANNEL         (0b00  << 6) 
#define PIT_COMMAND_VALUE (PIT_COMMAND_VALUE_BINARY_MODE | PIT_COMMAND_VALUE_OPR_SQUARE_WAVE | PIT_COMMAND_VALUE_ACC_LOHIBYTE | PIT_COMMAND_VALUE_CHANNEL)

#define PIT_CHANNEL_0_DATA_PIO 0x40

void activate_timer_interrupt(void) {
    __asm__ volatile("cli");
    // Setup how often PIT fire
    uint32_t pit_timer_counter_to_fire = PIT_TIMER_COUNTER;
    out(PIT_COMMAND_REGISTER_PIO, PIT_COMMAND_VALUE);
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t) (pit_timer_counter_to_fire & 0xFF));
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t) ((pit_timer_counter_to_fire >> 8) & 0xFF));

    // Activate the interrupt
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_TIMER));
   
}
