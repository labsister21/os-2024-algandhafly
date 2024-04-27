#include "header/interrupt/idt.h"
#include "header/cpu/gdt.h"
#include "header/text/stringdrawer.h"

// void *isr_stub_table[ISR_STUB_TABLE_LIMIT]; // defined in intsetup.s
struct InterruptDescriptorTable interrupt_descriptor_table;
struct IDTR _idt_idtr = {
    .size = sizeof(interrupt_descriptor_table),
    .address = &interrupt_descriptor_table,
};

void initialize_idt(void) {
    for (int i = 0; i < ISR_STUB_TABLE_LIMIT; i++) {
        uint8_t privilege = 0;
        if (i - 0x30) privilege = 0x3; 
        
        set_interrupt_gate(i, isr_stub_table[i], GDT_KERNEL_CODE_SEGMENT_SELECTOR, privilege);
    }

    __asm__ volatile("sti");
    __asm__ volatile("lidt %0" : : "m"(_idt_idtr));
}

void set_interrupt_gate(
    uint8_t  int_vector, 
    void     *handler_address, 
    uint16_t gdt_seg_selector, 
    uint8_t  privilege
) {
    struct IDTGate *idt_int_gate = &interrupt_descriptor_table.table[int_vector];
    idt_int_gate->offset_low  = (uint32_t)handler_address & 0xFFFF;
    idt_int_gate->segment     = gdt_seg_selector;
    idt_int_gate->_reserved   = 0;
    idt_int_gate->descriptor_privilege_level = privilege;
    idt_int_gate->offset_high = (uint32_t)handler_address >> 16 & 0xFFFF;

    // Target system 32-bit and flag this as valid interrupt gate
    idt_int_gate->_r_bit_1    = INTERRUPT_GATE_R_BIT_1;
    idt_int_gate->_r_bit_2    = INTERRUPT_GATE_R_BIT_2;
    idt_int_gate->_r_bit_3    = INTERRUPT_GATE_R_BIT_3;
    idt_int_gate->gate_32     = 1;
    idt_int_gate->valid_bit   = 1;
}
