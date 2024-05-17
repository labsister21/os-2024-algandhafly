#include <stdint.h>
#include <stdbool.h>

void systemCall__custom(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

// Approximation. Not reliable
void sleep__custom(uint64_t ms){
    uint64_t wait_time = 1e5 * ms;
    for(uint64_t i = 0; i < wait_time; i++){}
}

int main(void) {
    
    uint8_t color = 0;
    while(true){
        sleep__custom(200);
        systemCall__custom(17, 0, 79, color++);
        if(color == 16) color = 0;
    }

    return 0;
}
