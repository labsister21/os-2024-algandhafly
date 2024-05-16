#include "header/scheduler/scheduler.h"
#include <string.h>

struct Scheduler_Queue SchedulerQueue;

void process_context_switch(struct Context context) {}

void scheduler_enqueue_process(PCB* pcb) {

    struct SchedulerQueueNode newNode;
    newNode.pcb = pcb;
    newNode.next = NULL;

    if (SchedulerQueue.first == NULL) {
        SchedulerQueue.first = &newNode;
        SchedulerQueue.last = &newNode;
    }
    else {
        SchedulerQueue.last->next = &newNode;
        SchedulerQueue.last = &newNode;
    }
}

void scheduler_dequeue_process() {

    struct SchedulerQueueNode* ret = SchedulerQueue.first;
    if (SchedulerQueue.first == SchedulerQueue.last) {
        SchedulerQueue.first = NULL;
        SchedulerQueue.last = NULL;
    }
    else {
        SchedulerQueue.first = SchedulerQueue.first->next;
    }
    return ret;
}

PCB* get_current_running_process() {
    for (;;)
    {
        struct SchedulerQueueNode* ret = SchedulerQueue.first;
        if (ret == NULL) return NULL;
        else if (ret->pcb->metadata.state == UNUSED) scheduler_dequeue_process();
        else return ret->pcb;
    }
}

PCB* scheduler_invalidate_and_dequeue() {
    struct SchedulerQueueNode* ret = get_current_running_process();
    scheduler_dequeue_process();
    return ret->pcb;
}


/* --- Scheduler --- */
/**
 * Initialize scheduler before executing init process 
 */
void scheduler_init(void) {
    SchedulerQueue.first = NULL;
    SchedulerQueue.last = NULL;
}

void scheduler_save_context_to_current_running_pcb(struct Context context) {
    PCB* current_process = get_current_running_process();
    memcpy(&context, &current_process->context, sizeof(context));
}

__attribute__((noreturn)) void scheduler_switch_to_next_process(void){


    struct Context context;

    // __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(context.cpu.general.eax));
    // __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(context.cpu.general.ebx));
    // __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(context.cpu.general.ecx));
    // __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(context.cpu.general.edx));
    
    // __asm__ volatile("mov %0, %%edi" : /* <Empty> */ : "r"(context.cpu.index.edi));
    // __asm__ volatile("mov %0, %%esi" : /* <Empty> */ : "r"(context.cpu.index.esi));
    
    // __asm__ volatile("mov %0, %%ds" : /* <Empty> */ : "r"(context.cpu.segment.ds));
    // __asm__ volatile("mov %0, %%es" : /* <Empty> */ : "r"(context.cpu.segment.es));
    // __asm__ volatile("mov %0, %%fs" : /* <Empty> */ : "r"(context.cpu.segment.fs));
    // __asm__ volatile("mov %0, %%gs" : /* <Empty> */ : "r"(context.cpu.segment.gs));

    // __asm__ volatile("mov %0, %%ebp" : /* <Empty> */ : "r"(context.cpu.stack.ebp));
    // __asm__ volatile("mov %0, %%esp" : /* <Empty> */ : "r"(context.cpu.stack.esp));


    PCB* old_process = get_current_running_process();
    if (old_process == NULL) return ;

    scheduler_save_context_to_current_running_pcb(context);
    scheduler_dequeue_process();
    scheduler_enqueue_process(old_process);

    PCB* new_process = get_current_running_process();
    paging_use_page_directory(new_process->context.page_dir);
    process_context_switch(new_process->context);
}