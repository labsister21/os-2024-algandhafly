#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../header/memory/paging.h"
#include "header/process/process.h"
#include "header/stdlib/string.h"

#define ISNT !=
#define not !
#define and &&
#define or ||

int PD_empty_slots = 1023;

__attribute__((aligned(0x1000))) struct PageDirectory _paging_kernel_page_directory = {
    .table = {
        [0] = {
            .flags.present_bit      = 1,
            .flags.read_write       = 1,
            .flags.page_size_4mb    = 1,
            .lower_address          = 0,
        },
        [0x300] = {
            .flags.present_bit      = 1,
            .flags.read_write       = 1,
            .flags.page_size_4mb    = 1,
            .lower_address          = 0,
        },
    },
};

static struct PageManagerState page_manager_state = {
    .page_frame_map = {
        [0]                            = true,
        [1 ... PAGE_FRAME_MAX_COUNT-1] = false,
    },
    .free_page_frame_count = PAGE_FRAME_MAX_COUNT - 1
};

void update_page_directory_entry(
    struct PageDirectory *page_dir,
    void *physical_addr, 
    void *virtual_addr, 
    struct PageDirectoryEntryFlags flags
) {
    uint32_t page_index = ((uint32_t) virtual_addr >> 22) & 0x3FF;
    page_dir->table[page_index].flags          = flags;
    page_dir->table[page_index].lower_address = ((uint32_t) physical_addr >> 22) & 0x3FF;
    flush_single_tlb(virtual_addr);
}

void flush_single_tlb(void *virtual_addr) {
    asm volatile("invlpg (%0)" : /* <Empty> */ : "b"(virtual_addr): "memory");
}

/* --- Memory Management --- */
bool paging_allocate_check(uint32_t amount) {
    return amount <= page_manager_state.free_page_frame_count;
}


void* paging_allocate_user_page_frame(struct PageDirectory *page_dir) {
    
    // page_dir->table[0].flags.present_bit = 1;
    // page_dir->table[1].flags.present_bit = 1;
    // page_dir->table[2].flags.present_bit = 1;
    // page_dir->table[3].flags.present_bit = 1;
    // page_dir->table[4].flags.present_bit = 1;
    // page_dir->table[5].flags.present_bit = 1;
    // page_dir->table[6].flags.present_bit = 1;
    // page_dir->table[7].flags.present_bit = 1;
    // page_dir->table[8].flags.present_bit = 1;
    // page_dir->table[9].flags.present_bit = 1;
    // page_dir->table[10].flags.present_bit = 1;
    // page_dir->table[11].flags.present_bit = 1;
    // page_dir->table[12].flags.present_bit = 1;
    // page_dir->table[13].flags.present_bit = 1;

    uint32_t page_index = 0;
    while (page_dir->table[page_index].flags.present_bit ISNT 0) {
        page_index++;
    }

    uint16_t page_frame_index = 0;
    for(;;)
    {
        if (page_frame_index >= PAGE_FRAME_MAX_COUNT) return false;
        if (!page_manager_state.page_frame_map[page_frame_index]) break;
        else ++page_frame_index;
    }
    
    page_manager_state.page_frame_map[page_frame_index] = 1;
    page_manager_state.free_page_frame_count--;
    uint16_t physical_addr = page_frame_index;
    
    page_dir->table[page_index].flags.present_bit       = 1;
    page_dir->table[page_index].flags.read_write        = 1;
    page_dir->table[page_index].flags.user_supervisor   = 1;
    page_dir->table[page_index].flags.page_size_4mb     = 1;
    page_dir->table[page_index].lower_address           = physical_addr;

    void* virtual_addr = (void*)(page_index << 22);
    flush_single_tlb(virtual_addr);

    return virtual_addr;
}

bool paging_free_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {

    uint32_t page_index = ((uint32_t) virtual_addr >> 22) & 0x3FF;
    page_dir->table[page_index].flags.present_bit = 0;
    flush_single_tlb(virtual_addr);
    
    uint32_t page_frame_index = page_dir->table[page_index].lower_address;
    page_manager_state.page_frame_map[page_frame_index] = false;
    page_manager_state.free_page_frame_count++;

    return true;
}