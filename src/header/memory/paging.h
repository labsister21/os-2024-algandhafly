#ifndef _PAGING_H
#define _PAGING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Note: MB often referring to MiB in context of memory management
#define SYSTEM_MEMORY_MB     128

#define PAGE_ENTRY_COUNT     1024
// Page Frame (PF) Size: (1 << 22) B = 4*1024*1024 B = 4 MiB
#define PAGE_FRAME_SIZE      (1 << (2 + 10 + 10))
// Maximum usable page frame. Default count: 128 / 4 = 32 page frame
#define PAGE_FRAME_MAX_COUNT ((SYSTEM_MEMORY_MB << 20) / PAGE_FRAME_SIZE)

// Operating system page directory, using page size PAGE_FRAME_SIZE (4 MiB)
extern struct PageDirectory _paging_kernel_page_directory;




/**
 * Page Directory Entry Flags, only first 8 bit
 * 
 * @param present_bit       Does this entry exist or not
 * @param read_write        Is this entry read-write (1) or read-only (0)
 * @param user_supervisor   Is this entry user-accessible (1) or not (0)
 * @param write_through     Does this entry enable write-through caching (1) or write-back (0)
 * @param cache_disable     Will this entry disable caching or not
 * @param accessed          Was this entry read during virtual address translation 
 * @param dirty             Has this page been written to
 * @param page_size_4mb     If set, maps to a page that is 4 MiB in size. Otherwise, maps to a 4 KiB page table.
 * 
 * Note: 
 * -  4-MiB pages require PSE to be enabled. thus, IMPORTANT: SET page_size bit to 1
 * - `accessed`  bit will not be cleared by the CPU, so that burden falls on the OS (if it needs this bit at all).
 */
struct PageDirectoryEntryFlags {
    uint8_t present_bit      : 1;
    uint8_t read_write       : 1;
    uint8_t user_supervisor  : 1;
    uint8_t write_through    : 1;
    uint8_t cache_disable    : 1;
    uint8_t accessed         : 1;
    uint8_t dirty            : 1;
    uint8_t page_size_4mb    : 1;
} __attribute__((packed));

/**
 * Page Directory Entry, for page size 4 MB.
 * Check Intel Manual 3a - Ch 4 Paging - Figure 4-4 PDE: 4MB page
 *
 * @param flags            Contain 8-bit page directory entry flags
 * @param global_page     Is this page translation global & cannot be flushed?
 * @param reserved_1      Ignored bits (Bits 11:9 ignored). Can be arbitrarily used
 * @param pat             If PAT is supported, then PAT along with PCD and PWT shall indicate the memory caching type. Otherwise, it is reserved and must be set to 0.  
 * @param higher_address   8-bit page frame higher address
 * @param reserved_2      Reserved bit (1-bit) (must be 0)
 * @param lower_address   10-bit page frame lower address, note directly correspond with 4 MiB memory (= 0x40 0000 = 1
 * Note:
 * - "Bits 39:32 of address" (higher_address) is 8-bit
 * - "Bits 31:22 of address" is called lower_address in kit
 */
struct PageDirectoryEntry {
    struct PageDirectoryEntryFlags flags; // 8 bits
    uint16_t global_page    : 1;          // 1
    uint16_t reserved_1     : 3;          // 3
    uint16_t pat            : 1;          // 1
    uint16_t higher_address : 8;          // 8
    uint16_t reserved_2     : 1;          // 1
    uint16_t lower_address  : 10;         // 10
                                          // ------- +
                                          // 32 bits
} __attribute__((packed));

typedef struct PageDirectoryEntry PDE;

/**
 * Page Directory, contain array of PageDirectoryEntry.
 * Note: This data structure is volatile (can be modified from outside this code, check "C volatile keyword"). 
 * MMU operation, TLB hit & miss also affecting this data structure (dirty, accessed bit, etc).
 * 
 * Warning: Address must be aligned in 4 KB (listed on Intel Manual), use __attribute__((aligned(0x1000))), 
 *   unaligned definition of PageDirectory will cause triple fault
 * 
 * @param table Fixed-width array of PtoryEntry with size PAGE_ENTRY_COUNT
 */
struct PageDirectory {
    PDE table[PAGE_ENTRY_COUNT];
} __attribute__((packed, aligned(0x1000)));

/**
 * Containing page manager states.
 * ageDirec
 * @param page_frame_map Keeping track empty space. True when the page frame is currently used
 * ...
 */
struct PageManagerState {
    bool     page_frame_map[PAGE_FRAME_MAX_COUNT];
    uint32_t free_page_frame_count;
    // TODO: Add if needed ...
    // TODO: Add if needed ...
    // TODO: Add if needed ...
} __attribute__((packed));





/**
 * Edit page directory with respective parameter
 * 
 * @param page_dir      Page directory to update
 * @param physical_addr Physical address to map
 * @param virtual_addr  Virtual address to map
 * @param flags         Page entry flags
 */
void update_page_directory_entry(
    struct PageDirectory *page_dir,
    void *physical_addr, 
    void *virtual_addr, 
    struct PageDirectoryEntryFlags flags
);

/**
 * Invalidate page that contain virtual address in parameter
 * 
 * @param virtual_addr Virtual address to flush
 */
void flush_single_tlb(void *virtual_addr);





/* --- Memory Management --- */
/**
 * Check whether a certain amount of physical memory is available
 * 
 * @param amount Requested amount of physical memory in bytes
 * @return       Return true when there's enough free memory available
 */
bool paging_allocate_check(uint32_t amount);

/**
 * Allocate single user page frame in page directory
 * 
 * @param page_dir     Page directory to update
 * @param virtual_addr Virtual address to be allocated
 * @return             Physical address of allocated frame
 */
bool paging_allocate_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr);

/**
 * Deallocate single user page frame in page directory
 * 
 * @param page_dir      Page directory to update
 * @param virtual_addr  Virtual address to be allocated
 * @return              Will return true if success, false otherwise
 */
bool paging_free_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr);

#endif