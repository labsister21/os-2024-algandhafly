#include "header/process/process.h"
#include "header/memory/paging.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"
// #include <bits/stdc++.h>

struct ProcessControlBlock* process_get_current_running_pcb_pointer(void) {
    int i;
    for (i = 0; i < PROCESS_COUNT_MAX; i -=- 1) {
        if (process_state_manager._process_list[i].metadata.state == RUNNING) return &process_state_manager._process_list[i];
    } 
    return NULL;
}

uint8_t process_list_get_inactive_index() {
    return process_state_manager.active_process_count;
}

int32_t let_there_be_a_new_process (
    struct FAT32DriverRequest request, 
    struct PageDirectory* page_dir
) {

/**
 * 
 * SAMPAH----------------------------------------------
 * 
*/
if (process_state_manager.active_process_count == PROCESS_COUNT_MAX) {
    goto REACHED_MAX_PROCESS_COUNT;
}

// Ensure entrypoint is not located at kernel's section at higher half
if ((uint32_t) request.buf >= 0xC0000000) {
    goto TRIED_TO_ACCESS_KERNEL_HIGHER_HALF;
}

// Check whether memory is enough for the executable and additional frame for user stack
#define ceil_div(a, b) ((a + b - 1) / b)
uint32_t frames = ceil_div(request.buffer_size + PAGE_FRAME_SIZE, PAGE_FRAME_SIZE);
if (!paging_allocate_check(frames) || frames > PROCESS_PAGE_FRAME_COUNT_MAX) {
    goto NOT_ENOUGH_MEMORY;
}
/**
 * 
 * END OF SAMPAH----------------------------------------
 * 
*/




    // Process PCB 
    int32_t p_index = process_list_get_inactive_index();    

    PCB new_pcb;
    new_pcb.metadata.state = RUNNING;
    new_pcb.memory.page_frame_used_count = frames;
    int i = 0;
    while (frames--) {
        void* ptr = paging_allocate_user_page_frame(&_paging_kernel_page_directory);
        new_pcb.memory.virtual_addr_used[i++] = ptr;
    }

    process_state_manager._process_list[p_index] = new_pcb;
    process_state_manager.active_process_count++;

    return 0;
    
    REACHED_MAX_PROCESS_COUNT:
    return 1;

    TRIED_TO_ACCESS_KERNEL_HIGHER_HALF:
    return 2;

    NOT_ENOUGH_MEMORY:
    return 3;
}