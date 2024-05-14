#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "header/process/process.h"

#define ISNT !=
#define not !
#define and &&
#define or ||
#define repeat \
        do {
#define until(condition) \
        } while (!(condition));

/**
 * Queue stuff
*/

struct SchedulerQueueNode {
    PCB* pcb;
    struct SchedulerQueueNode* next;
};

static struct {
    struct SchedulerQueueNode* first;
    struct SchedulerQueueNode* last;
} SchedulerQueue;

/**
 * Enqueue, dequeue, and get_first
 * Note: invalidate_and_dequeue and get_first will keep unaliving the first process of the queue if the process is marked as killed until it finds a process that isn't killed
 * 
*/
void scheduler_enqueue_process(PCB*);

void scheduler_dequeue_process();

PCB* get_current_running_process();

PCB* scheduler_invalidate_and_dequeue();


/**
 * Read all general purpose register values and set control register.
 * Resume the execution flow back to ctx.eip and ctx.eflags
 * 
 * @note          Implemented in assembly
 * @param context Target context to switch into
 */
__attribute__((noreturn)) extern void process_context_switch(struct Context context);



/* --- Scheduler --- */
/**
 * Initialize scheduler before executing init process 
 */
void scheduler_init(void); 

/**
 * Save context to current running process
 * 
 * @param ctx Context to save to current running process control block
 */
void scheduler_save_context_to_current_running_pcb(struct Context context);

/**
 * Trigger the scheduler algorithm and context switch to new process
 */
__attribute__((noreturn)) void scheduler_switch_to_next_process(void);

#endif