#ifndef COMMAND_H
#define COMMAND_H
#include <directorystack.h>

uint8_t path_to_dir_stack(char* path, struct DirectoryStack* dir_stack);
void command(char *buf, struct DirectoryStack* dir_stack);


#endif