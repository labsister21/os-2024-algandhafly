#ifndef _IDT_H
#define _IDT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// IDT hard limit, see Intel x86 manual 3a - 6.10 Interrupt Descriptor Table
#define IDT_MAX_ENTRY_COUNT    256
#define ISR_STUB_TABLE_LIMIT   64
#define INTERRUPT_GATE_R_BIT_1 0b000
#define INTERRUPT_GATE_R_BIT_2 0b110
#define INTERRUPT_GATE_R_BIT_3 0b0

// Interrupt Handler / ISR stub for reducing code duplication, this array can be iterated in initialize_idt()
extern void *isr_stub_table[ISR_STUB_TABLE_LIMIT];

extern struct IDTR _idt_idtr;

/**
 * IDTGate, IDT entry that point into interrupt handler
 * Struct defined exactly in Intel x86 Vol 3a - Figure 6-2. IDT Gate Descriptors
 *
 * @param offset_low  Lower 16-bit offset
 * @param segment     Memory segment
 * @param _reserved   Reserved bit, bit length: 5
 * @param _r_bit_1    Reserved for idtgate type, bit length: 3
 * @param _r_bit_2    Reserved for idtgate type, bit length: 3
 * @param gate_32     Is this gate size 32-bit? If not then its 16-bit gate
 * @param _r_bit_3    Reserved for idtgate type, bit length: 1
 * ...
 */
struct IDTGate {
    // First 32-bit (Bit 0 to 31)
    uint16_t offset_low;
    uint16_t segment;

    // TODO : Implement


    // Next 16-bit (Bit 32 to 47)
    uint8_t _reserved: 5;
    uint8_t _r_bit_1: 3;
    uint8_t _r_bit_2: 3;
    uint8_t descriptor_privilege_level : 2;     // DPL
    uint8_t segment_present            : 1;     // P

    // Next 16-bit (Bit 48 to 63)
    uint16_t gate_32;

    // ??? see idt.c
    uint8_t _r_bit_3: 3;
    uint8_t valid_bit: 1;



} __attribute__((packed));

/**
 * Interrupt Descriptor Table, containing lists of IDTGate.
 * One IDT already defined in idt.c
 *
 * ...
 */
// TODO : Implement
// ...
struct ListIDTGate {
    struct IDTGate* table;
};
extern struct ListIDTGate interrupt_descriptor_table;


/**
 * IDTR, carrying information where's the IDT located and size.
 * Global kernel variable defined at idt.c.
 *
 * ...
 */
// TODO : Implement
// ...
struct IDTR {
    uint16_t size;
    uint32_t offset;
} __attribute__((packed));



/**
 * Set IDTGate with proper interrupt handler values.
 * Will directly edit global IDT variable and set values properly
 * 
 * @param int_vector       Interrupt vector to handle
 * @param handler_address  Interrupt handler address
 * @param gdt_seg_selector GDT segment selector, for kernel use GDT_KERNEL_CODE_SEGMENT_SELECTOR
 * @param privilege        Descriptor privilege level
 */
void set_interrupt_gate(uint8_t int_vector, void *handler_address, uint16_t gdt_seg_selector, uint8_t privilege);


/**
 * Set IDT with proper values and load with lidt
 */
void initialize_idt(void);

#endif