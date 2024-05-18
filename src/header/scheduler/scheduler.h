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

PCB* get_current_running_process();

extern void fly_to_the_sky(struct Context context);
void scheduler_init();

// __attribute__((noreturn));
__attribute__((noreturn)) void scheduler_switch_to_next_process(struct InterruptFrame frame);

#endif