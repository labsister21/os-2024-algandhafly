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
#define God_PageDir _paging_kernel_page_directory
typedef struct PageDirectory PD;

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
 * 
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
 *
 * @param flags           Contain 8-bit page directory entry flags
 * @param global_page     Is this page translation global & cannot be flushed?
 * @param reserved_1      Ignored bits (Bits 11:9 ignored). Can be arbitrarily used
 * @param pat             If PAT is supported, then PAT along with PCD and PWT shall indicate the memory caching type. Otherwise, it is reserved and must be set to 0.  
 * @param higher_address   8-bit page frame higher address
 * @param reserved_2      Reserved bit (1-bit) (must be 0)
 * @param lower_address   10-bit page frame lower address
 * 
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
volatile struct PageDirectory {
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
} __attribute__((packed));


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
bool paging_allocate_user_page_frame(struct PageDirectory *page_dir, void* virtual_addr);

/**
 * Deallocate single user page frame in page directory
 * 
 * @param page_dir      Page directory to update
 * @param virtual_addr  Virtual address to be allocated
 * @return              Will return true if success, false otherwise
 */
bool paging_free_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr);

/* --- Process-related Memory Management --- */
#define PAGING_DIRECTORY_TABLE_MAX_COUNT 32

/**
 * Create new page directory prefilled with 1 page directory entry for kernel higher half mapping
 * 
 * @return Pointer to page directory virtual address. Return NULL if allocation failed
 */
struct PageDirectory* paging_create_new_page_directory(void);

/**
 * Free page directory and delete all page directory entry
 * 
 * @param page_dir Pointer to page directory virtual address
 * @return         True if free operation success 
 */
bool paging_free_page_directory(struct PageDirectory *page_dir);

/**
 * Get currently active page directory virtual address from CR3 register
 * 
 * @note   Assuming page directories lives in kernel memory
 * @return Page directory virtual address currently active (CR3)
 */
struct PageDirectory* paging_get_current_page_directory_addr(void);

/**
 * Change active page directory (indirectly trigger TLB flush for all non-global entry)
 * 
 * @note                        Assuming page directories lives in kernel memory
 * @param page_dir_virtual_addr Page directory virtual address to switch into
 */
void paging_use_page_directory(struct PageDirectory *page_dir_virtual_addr);


#endif