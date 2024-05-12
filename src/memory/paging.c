#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../header/memory/paging.h"
#include "header/process/process.h"
#include "header/stdlib/string.h"

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
    uint32_t BYTES = page_manager_state.free_page_frame_count * PAGE_FRAME_SIZE;
    return BYTES >= amount;
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
    /*
     * TODO: Get & initialize empty page directory from page_directory_list
     * - Iterate page_directory_list[] & get unused page directory
     * - Mark selected page directory as used
     * - Create new page directory entry for kernel higher half with flag:
     *     > present bit    true
     *     > write bit      true
     *     > pagesize 4 mb  true
     *     > lower address  0
     * - Set page_directory.table[0x300] with kernel page directory entry
     * - Return the page directory address
     */
    
    uint16_t i;

    for (i = 0; i < PAGING_DIRECTORY_TABLE_MAX_COUNT; i++) {
        if (!page_directory_manager.page_directory_used[i]) break;
    }
    page_directory_manager.page_directory_used[i] = 1;

    if (i == PAGING_DIRECTORY_TABLE_MAX_COUNT) return NULL;

    else;

        __attribute__((aligned(0x1000))) struct PageDirectory paging_kernel_page_directory = {
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
        page_directory_list[i] = paging_kernel_page_directory;
        return &page_directory_list[i];
}

bool paging_free_page_directory(struct PageDirectory *page_dir) {
    /**
     * TODO: Iterate & clear page directory values
     * - Iterate page_directory_list[] & check &page_directory_list[] == page_dir
     * - If matches, mark the page directory as unusued and clear all page directory entry
     * - Return true
     */
    for(int i=0; i < PAGE_FRAME_MAX_COUNT;i++) {
        if(&page_directory_list[i] == page_dir){
            
        }
    }
    return false;
}

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