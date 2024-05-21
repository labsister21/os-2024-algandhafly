#include <stdint.h>
#include <stdbool.h>

void systemCall__clock(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

// Approximation. Not reliable
void sleep__clock(uint64_t ms){
    uint64_t wait_time = 1e5 * ms;
    for(uint64_t i = 0; i < wait_time; i++){}
}

int main(void) {
    
    while(true){
        sleep__clock(200);
        systemCall__clock(15, 0, 0, 0);
    }

    return 123;
}
