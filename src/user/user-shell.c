#include <stdint.h>
#include <stdbool.h>
#include "../header/filesystem/fat32.h"

void systemCall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}


int main(void) {
    // __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(0xDEADBEEF));

    // struct ClusterBuffer      cl[2]   = {0};
    // struct FAT32DriverRequest request = {
    //     .buf                   = &cl,
    //     .name                  = "shell",
    //     .ext                   = "\0\0\0",
    //     .parent_cluster_number = ROOT_CLUSTER_NUMBER,
    //     .buffer_size           = CLUSTER_SIZE,
    // };
    // int32_t retcode;
    // systemCall(0, (uint32_t) &request, (uint32_t) &retcode, 0);
    // if (retcode == 0)
    //     systemCall(6, (uint32_t) "owo\n", 4, 0xF);

    char *buf = "ayam";
    // systemCall(7, 0, 0, 0);
    buf[0]++;
    systemCall(5, (uint32_t) buf, 0, 0);
    while (true) {
        // systemCall(4, (uint32_t) &buf, 0, 0);
        buf[0]++;
        systemCall(5, (uint32_t) buf, 0, 0);
    }


    return 0;
}
