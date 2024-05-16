#include "header/scheduler/scheduler.h"

int a_certain_magical_index = 0;

PCB* get_current_running_process() {
    return &_process_list[a_certain_magical_index];
}

// __attribute__((noreturn));
 void scheduler_switch_to_next_process(void) {
    
    PCB* old_pcb = get_current_running_process();

    repeat { a_certain_magical_index = (a_certain_magical_index + 1) % PROCESS_COUNT_MAX; } 
    
    until (_process_list[a_certain_magical_index].metadata.state == RUNNING);

    PCB* new_pcb = get_current_running_process();

    paging_use_page_directory(new_pcb->context.page_dir);

    /**
     * 
     * @todo pindahin context to old_pcb, fly to the sky
     * 
    */
    
    // asm("\t movl %%ebp,%0" : "=r"(old_pcb->context.cpu.stack.ebp));
    // asm("\t movl %%esp,%0" : "=r"(old_pcb->context.cpu.stack.esp));
    
    // asm("\t movl %%edi,%0" : "=r"(old_pcb->context.cpu.index.edi));
    // asm("\t movl %%esi,%0" : "=r"(old_pcb->context.cpu.index.esi));
    
    // asm("\t movl %%ds,%0" : "=r"(old_pcb->context.cpu.segment.ds));
    // asm("\t movl %%es,%0" : "=r"(old_pcb->context.cpu.segment.es));
    // asm("\t movl %%fs,%0" : "=r"(old_pcb->context.cpu.segment.fs));
    // asm("\t movl %%gs,%0" : "=r"(old_pcb->context.cpu.segment.gs));
    
    ;
    // asm("\t movl %%eip,%0" : "=r"(old_pcb->context.eip));

    // __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(context.cpu.general.eax));

    // fly_to_the_sky(new_pcb->context);
}