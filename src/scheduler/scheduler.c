#include "header/scheduler/scheduler.h"
#include "header/kernel-entrypoint.h"

int a_certain_magical_index = 0;


PCB* get_current_running_process() {
    return &_process_list[a_certain_magical_index];
}

void scheduler_init() {
    struct Context context;

    PCB* old_pcb = get_current_running_process();
    
    paging_use_page_directory(old_pcb->context.page_dir);

    context = old_pcb->context;
    fly_to_the_sky(context);
}

__attribute__((noreturn)) void scheduler_switch_to_next_process(struct InterruptFrame frame) {

    struct Context context;

    PCB* old_pcb = get_current_running_process();

    repeat { a_certain_magical_index = (a_certain_magical_index + 1) % PROCESS_COUNT_MAX; } 
    
    until (_process_list[a_certain_magical_index].metadata.state == RUNNING);

    PCB* new_pcb = get_current_running_process();

    
    paging_use_page_directory(new_pcb->context.page_dir);

    old_pcb->context.eflags = frame.int_stack.eflags;
    old_pcb->context.eip = frame.int_stack.eip;
    old_pcb->context.cs = frame.int_stack.cs;
    old_pcb->context.cpu = frame.cpu;

    context = new_pcb->context;
    pic_ack(IRQ_TIMER);
    fly_to_the_sky(context);
}