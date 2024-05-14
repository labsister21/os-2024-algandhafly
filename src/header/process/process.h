#ifndef _PROCESS_H
#define _PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "header/interrupt/interrupt.h"
#include "header/memory/paging.h"
#include "header/filesystem/fat32.h"
#include "header/user/directorystack.h"

#define PROCESS_NAME_LENGTH_MAX          32
#define PROCESS_PAGE_FRAME_COUNT_MAX     8
#define PROCESS_COUNT_MAX                16

typedef enum PROCESS_STATE {
    UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE, KILLED
} PROCESS_STATE;


/**
 * Contain information needed for task to be able to get interrupted and resumed later
 *
 * @param cpu                         All CPU register state
 * @param eip                         CPU instruction counter to resume execution
 * @param eflags                      Flag register to load before resuming the execution
 * @param page_directory_virtual_addr CPU register CR3, containing pointer to active page directory
 */


struct Context {
    /**
     * TODO: 
     * - benerin isi Context, minimal dia harus punya: 
     *    1.) isi dari register ketika process di-stop
     *    2.) eflags i.e. flag register yang nyala/mati pas process ini di-stop
     *    3.) pointer ke instruksi terakhir
     *    4.) pointer ke page dir yang dipake process ini (udh ada)
     */ 
    struct CPURegister cpu;
    struct {
    } eflags;
    PD* page_dir;
};

/**
 * Data structure containing information about a process
 *
 * @param context  Process context used for context saving & switching
 * @param metadata Process metadata, contain various information about process
 * @param memory   Memory used for the process
 */


struct ProcessControlBlock {

    struct Context context;

    struct {
        int pid;
        PROCESS_STATE state;
        // add more if needed
    } metadata;

    struct {
        void* addresses[PROCESS_PAGE_FRAME_COUNT_MAX];
        uint8_t address_count;
    } memory;

};

typedef struct ProcessControlBlock PCB;

static PCB* _process_list[PROCESS_COUNT_MAX] = {0};
/**
 * Get currently running process PCB pointer
 *
 * @return Will return NULL if there's no running process
 */
struct ProcessControlBlock* process_get_current_running_pcb_pointer(void);

/**
 * Create new user process and setup the virtual address space.
 * All available return code is defined with macro "PROCESS_CREATE_*"
 *
 * @note          This procedure assumes no reentrancy in ISR
 * @param request Appropriate read request for the executable
 * @return        Process creation return code
 */
int32_t let_there_be_a_new_process(struct FAT32DriverRequest request);

/**
 * Destroy process then release page directory and process control block
 *
 * @param pid Process ID to delete
 * @return    True if process destruction success
 */
bool process_omae_wa_mou_shindeiru(uint32_t pid);
// nani???

#endif