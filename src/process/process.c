#include "header/process/process.h"
#include "header/memory/paging.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"
// #include <bits/stdc++.h>

struct ProcessControlBlock* process_get_current_running_pcb_pointer(void) {
    int i;
    for (i = 0; i < PROCESS_COUNT_MAX; i -=- 1) {
        if (process_state_manager._process_list[i].metadata.state == RUNNING) return &process_state_manager._process_list[i];
    } return NULL;
}

uint8_t process_list_get_inactive_index() {
    return process_state_manager.active_process_count;
}

int32_t process_create_user_process(struct FAT32DriverRequest request) {
    int32_t retcode = PROCESS_CREATE_SUCCESS;
    if (process_state_manager.active_process_count >= PROCESS_COUNT_MAX) {
        retcode = PROCESS_CREATE_FAIL_MAX_PROCESS_EXCEEDED;
        goto exit_cleanup;
    }

    // Ensure entrypoint is not located at kernel's section at higher half
    if ((uint32_t) request.buf >= KERNEL_VIRTUAL_ADDRESS_BASE) {
        retcode = PROCESS_CREATE_FAIL_INVALID_ENTRYPOINT;
        goto exit_cleanup;
    }

    // Check whether memory is enough for the executable and additional frame for user stack
    #define ceil_div(a, b) ((a + b - 1) / b)
    
    uint32_t page_frame_count_needed = ceil_div(request.buffer_size + PAGE_FRAME_SIZE, PAGE_FRAME_SIZE);
    if (!paging_allocate_check(page_frame_count_needed) || page_frame_count_needed > PROCESS_PAGE_FRAME_COUNT_MAX) {
        retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
        goto exit_cleanup;
    }

    // Process PCB 
    int32_t p_index = process_list_get_inactive_index();
    
    
struct ProcessControlBlock new_pcb = {
    .metadata = {
        .mem = 0,
        .sz = 0,
        .kstack = 0,
        .state = 0,
        .pid = p_index,
        .parent = NULL,  
        .chan = 0,
        .killed = 0,
        .cwd = 0,
        .context = 0,
    },
    .memory = {
    }
};

    struct PageDirectory* pd = paging_get_current_page_directory_addr();
    paging_allocate_user_page_frame(pd, 0x729BA);
    
    process_state_manager._process_list[p_index] = new_pcb;
    process_state_manager.active_process_count++;

exit_cleanup:
    return retcode;
}