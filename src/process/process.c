#include "header/process/process.h"
#include "header/memory/paging.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"
#include "header/scheduler/scheduler.h"

PCB _process_list[PROCESS_COUNT_MAX] = {0};

void initialize_process_list() {
    for (int i = 0; i < PROCESS_COUNT_MAX; i++) {
        _process_list[i].metadata.state = UNUSED;
    } 
};

uint8_t process_list_get_inactive_index() {
    for (int i = 0; i < PROCESS_COUNT_MAX; i++) {
        if (_process_list[i].metadata.state == UNUSED) {
            return i;
        }
    }
    return -1;
}

int32_t let_there_be_a_new_process (
    struct FAT32DriverRequest request 
) {

/**
 * 
 * SAMPAH----------------------------------------------
 * 
*/

// CHECK PROCESS COUNT
int32_t p_index = process_list_get_inactive_index();
if (p_index == -1) goto REACHED_MAX_PROCESS_COUNT;

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

    PD* old_page_dir = paging_get_current_page_directory_addr();
    PD* page_dir = paging_create_new_page_directory();
    if (page_dir == NULL) goto NULL_PAGE_DIR;

    int i;
    void* virtual_addr = request.buf;

    // PCB information initialization
    _process_list[p_index].metadata.state = RUNNING;
    _process_list[p_index].memory.address_count = frames;

    // allocate frames in page_dir
    for (i = 0; i < frames; i++) {
        if (!paging_allocate_user_page_frame(page_dir, virtual_addr + (i << 22))) {
            goto CANT_FIT_MEMORY; // kalo nyampe ke sini, ganti request.buf
        }
        else {
            _process_list[p_index].memory.addresses[i] = virtual_addr + (i << 22);
        }
    }

    paging_use_page_directory(page_dir);
    read(request);
    paging_use_page_directory(old_page_dir);

    uint32_t CPU_EFLAGS_BASE_FLAG = 0x2;
    uint32_t CPU_EFLAGS_FLAG_INTERRUPT_ENABLE = 0x200;

    _process_list[p_index].context.eflags |= CPU_EFLAGS_BASE_FLAG | CPU_EFLAGS_FLAG_INTERRUPT_ENABLE;

    _process_list[p_index].context.page_dir = page_dir;
    _process_list[p_index].metadata.pid = p_index;

    memcpy(_process_list[p_index].metadata.name, request.name, 8);
    memcpy(_process_list[p_index].metadata.ext, request.ext, 3);

    return 0;
    
    REACHED_MAX_PROCESS_COUNT:
    return 1;

    TRIED_TO_ACCESS_KERNEL_HIGHER_HALF:
    return 2;

    NOT_ENOUGH_MEMORY:
    return 3;

    CANT_FIT_MEMORY:
    return 4;

    REQUEST_READ_FAILED:
    return 5;

    NULL_PAGE_DIR:
    return 6;
}

/*
    ini skema alokasi yang bener harusnya, kalau ada multiple process for a single program, kita kan gabisa main tentuin aja alokasi dari 0. tapi berhubung user program selalu di mulai dari 0 yo wes gausa dipake ini


    // TRY TO ALLOCATE CONTIGUOUS FRAME ENTRIES IN page_dir AT THE VIRTUAL ADDRESSES 
    // [i << 22 ... (i + frames) << 22].
    // IF ONE FAILS, BACK TRACK AND FREE ALL SUCCESSFULLY ALLOCATED ALLOCATED FRAMES AND TRY AGAIN AT NEXT i

    for (int j = 0; j < PAGE_ENTRY_COUNT - frames; j++) {
        bool succeed = true;
        for (i = j; i < frames; i++) {
            
            if (!paging_allocate_user_page_frame(page_dir, virtual_addr + (i << 22))) {
                succeed = false;
                for (int k = i - 1; k >= j; k--) {
                    paging_free_user_page_frame(page_dir, virtual_addr);
                }
                j = i + 1;
                break;
            }
        
        }
        if (succeed) break;
    }
*/

bool process_omae_wa_mou_shindeiru(uint32_t pid) {
    // nani???

    int idx = pid;
    
    if (_process_list[idx].metadata.state == UNUSED) return 1;
    
    else ;
        PCB* process = &_process_list[idx];
        _process_list[idx].metadata.state = UNUSED;
        PD* page_dir = process->context.page_dir;
        paging_free_page_directory(page_dir);
        return 1;
}