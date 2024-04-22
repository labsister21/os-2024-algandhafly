#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../header/memory/paging.h"

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
    }
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
    uint32_t bytes = page_manager_state.free_page_frame_count * PAGE_FRAME_SIZE;
    return bytes >= amount;
}


bool paging_allocate_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    
    uint32_t page_index = ((uint32_t) virtual_addr >> 22) & 0x3FF;
    if (page_dir->table[page_index].flags.present_bit) return false;

    uint32_t page_frame_index = 0;
    for(;;)
    {
        if (page_frame_index > PAGE_FRAME_MAX_COUNT) return false;
        if (page_manager_state.page_frame_map[page_frame_index]) break;
        else ++page_frame_index;
    }
    
    page_manager_state.page_frame_map[page_frame_index] = 1;
    page_manager_state.free_page_frame_count--;
    uint32_t physical_addr = page_frame_index;

    page_dir->table[page_index].flags.present_bit       = 1;
    page_dir->table[page_index].flags.read_write        = 1;
    page_dir->table[page_index].flags.user_supervisor   = 1;
    page_dir->table[page_index].flags.page_size_4mb     = 1;
    page_dir->table[page_index].lower_address           = physical_addr;

    return true;
}

bool paging_free_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {

    uint32_t page_index = (uint32_t) virtual_addr >> 22;
    page_dir->table[page_index].flags.present_bit = 0;
    
    uint32_t physical_addr = page_dir->table[page_index].lower_address;
    uint32_t page_frame_index = (physical_addr >> 22) & 0x1F;
    page_manager_state.page_frame_map[page_frame_index] = false;
    page_manager_state.free_page_frame_count--;

    return true;
}