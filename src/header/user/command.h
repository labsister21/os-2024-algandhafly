#ifndef COMMAND_H
#define COMMAND_H
#include <cwdlist.h>

void command(char *buf, struct CWDList* cwd_list);
void init_user_driver_state();

#endif