#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

void systemCall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

// Codes:
// 0: Read
// 1: Read Directory
// 2: Write
// 3: Delete
// 4: Get Keyboard Buffer
// 5: Kernel Puts
// 6: Kernel Gets
// 7: Keyboard State Activate

#endif
