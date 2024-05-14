#include "header/scheduler/scheduler.h"
#include <string.h>

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
        if (ret->pcb->metadata.state == KILLED) scheduler_dequeue_process();
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

    struct Context context = {
        // gua masih gatau cara get current context
    };
    scheduler_save_context_to_current_running_pcb(context);
    PCB* old_process = scheduler_invalidate_and_dequeue();
    scheduler_enqueue_process(old_process);

    PCB* new_process = get_current_running_process();
    process_context_switch(new_process->context);
}