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
#define repeat \
        do {
#define until(condition) \
        } while (!(condition));


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

__attribute__((aligned(0x1000))) static struct PageDirectory page_directory_list[PAGING_DIRECTORY_TABLE_MAX_COUNT] = {0};

static struct {
    bool page_directory_used[PAGING_DIRECTORY_TABLE_MAX_COUNT];
} _pagedir_manager = {
    .page_directory_used = {false},
};


void flush_single_tlb(void *virtual_addr) {
    asm volatile("invlpg (%0)" : /* <Empty> */ : "b"(virtual_addr): "memory");
}

/* --- Memory Management --- */
bool paging_allocate_check(uint32_t amount) {
    return amount <= page_manager_state.free_page_frame_count;
}


bool paging_allocate_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    
    uint32_t page_index = ((uint32_t) virtual_addr >> 22) & 0x3FF;
    if (page_dir->table[page_index].flags.present_bit) return false;

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

    flush_single_tlb(virtual_addr);

    return true;
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

struct PageDirectory* paging_create_new_page_directory(void) {
    
    // find empty page dir slot
    int i = -1;
    repeat { i++; } until (
        i == PAGING_DIRECTORY_TABLE_MAX_COUNT 
        or 
        _pagedir_manager.page_directory_used[i] == false
    ); 
    if (i == PAGING_DIRECTORY_TABLE_MAX_COUNT) return NULL;

    // flag the entry in bool array as used.
    _pagedir_manager.page_directory_used[i] = 1; 

    // mark the entry flags for kernel higher half in the to-use page dir.
    page_directory_list[i].table[0x300].flags.present_bit = 1;
    page_directory_list[i].table[0x300].flags.read_write = 1;
    page_directory_list[i].table[0x300].flags.page_size_4mb = 1;
    page_directory_list[i].table[0x300].lower_address = 0;

    return &page_directory_list->table[i];
}

bool paging_free_page_directory(struct PageDirectory *page_dir) {

    // mark all pd entries as not present in this page_dir
    int i = 0;
    repeat {
        page_dir->table[i].flags.present_bit = 0;
        page_dir->table[i].flags.page_size_4mb = 0; // probably gausah but who knows?
    } until (++i == PAGE_ENTRY_COUNT)

    // find corresponding page_dir in list
    i = -1;
    repeat { i++; } until (
        i == PAGING_DIRECTORY_TABLE_MAX_COUNT 
        or 
        &page_directory_list[i] == page_dir
    ); 
    if (i == PAGING_DIRECTORY_TABLE_MAX_COUNT) return false;

    // mark the entry in bool array as not used.
    _pagedir_manager.page_directory_used[i] = 0;

    return true;
}


/**
 * senjata tempur yang telah ditempa oleh almighty garbage collector
*/

#define KERNEL_VIRTUAL_ADDRESS_BASE 0xc0000000

struct PageDirectory* paging_get_current_page_directory_addr(void) {
    uint32_t current_page_directory_phys_addr;
    __asm__ volatile("mov %%cr3, %0" : "=r"(current_page_directory_phys_addr): /* <Empty> */);
    uint32_t virtual_addr_page_dir = current_page_directory_phys_addr + KERNEL_VIRTUAL_ADDRESS_BASE;
    return (struct PageDirectory*) virtual_addr_page_dir;
}

void paging_use_page_directory(struct PageDirectory *page_dir_virtual_addr) {
    uint32_t physical_addr_page_dir = (uint32_t) page_dir_virtual_addr;
    // Additional layer of check & mistake safety net
    if ((uint32_t) page_dir_virtual_addr > KERNEL_VIRTUAL_ADDRESS_BASE)
        physical_addr_page_dir -= KERNEL_VIRTUAL_ADDRESS_BASE;
    __asm__  volatile("mov %0, %%cr3" : /* <Empty> */ : "r"(physical_addr_page_dir): "memory");
}