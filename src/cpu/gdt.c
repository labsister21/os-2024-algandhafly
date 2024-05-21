#include "header/cpu/gdt.h"
#include "header/interrupt/interrupt.h"

/**
 * global_descriptor_table, predefined GDT.
 * Initial SegmentDescriptor already set properly according to Intel Manual & OSDev.
 * Table entry : [{Null Descriptor}, {Kernel Code}, {Kernel Data (variable, etc)}, {User Code}, {User Data}, {TSS}].
 */
struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        { 
            .segment_low = 0,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0,
            .non_system = 0,
            .descriptor_privilege_level = 0,
            .segment_present = 0,
            .segment_limit_high = 0,
            .available = 0,
            .code_segment = 0,
            .default_operation_size = 0,
            .granularity = 0,
            .base_high = 0
        },
        // Kernel
        {
            .segment_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0b1010,
            .non_system = 1,
            .descriptor_privilege_level = 0,
            .segment_present = 1,
            .segment_limit_high = 0xF,
            .available = 0,
            .code_segment = 0,
            .default_operation_size = 1,
            .granularity = 1,
            .base_high = 0
        },
        {
            .segment_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0b0010,
            .non_system = 1,
            .descriptor_privilege_level = 0,
            .segment_present = 1,
            .segment_limit_high = 0xF,
            .available = 0,
            .code_segment = 0,
            .default_operation_size = 1,
            .granularity = 1,
            .base_high = 0
        }, 
        // User 
        {
            .segment_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0b1010,
            .non_system = 1,
            .descriptor_privilege_level = 3,
            .segment_present = 1,
            .segment_limit_high = 0xF,
            .available = 0,
            .code_segment = 0,
            .default_operation_size = 1,
            .granularity = 1,
            .base_high = 0
        },
        {
            .segment_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0b0010,
            .non_system = 1,
            .descriptor_privilege_level = 3,
            .segment_present = 1,
            .segment_limit_high = 0xF,
            .available = 0,
            .code_segment = 0,
            .default_operation_size = 1,
            .granularity = 1,
            .base_high = 0
        }, 
        // TSS
        {
            .segment_limit_high     = (sizeof(struct TSSEntry) & (0xF << 16)) >> 16,
            .segment_low       = sizeof(struct TSSEntry),
            .base_high         = 0,
            .base_mid          = 0,
            .base_low          = 0,
            .non_system        = 0,    // S bit
            .type_bit          = 0x9,
            .descriptor_privilege_level        = 0,    // DPL
            .segment_present         = 1,    // P bit
            .default_operation_size        = 1,    // D/B bit
            .code_segment         = 0,    // L bit
            .granularity       = 0,    // G bit
        },
        {0}
    }
};

void gdt_install_tss(void) {
    uint32_t base = (uint32_t) &_interrupt_tss_entry;
    global_descriptor_table.table[5].base_high = (base & (0xFF << 24)) >> 24;
    global_descriptor_table.table[5].base_mid  = (base & (0xFF << 16)) >> 16;
    global_descriptor_table.table[5].base_low  = base & 0xFFFF;
}


/**
 * _gdt_gdtr, predefined system GDTR. 
 * GDT pointed by this variable is already set to point global_descriptor_table above.
 * From: https://wiki.osdev.org/Global_Descriptor_Table, GDTR.size is GDT size minus 1.
 */
struct GDTR _gdt_gdtr = {
    // TODO : Implement, this GDTR will point to global_descriptor_table. 
    //        Use sizeof operator
    .size = sizeof(struct GlobalDescriptorTable)-1,
    .address = &global_descriptor_table
};
