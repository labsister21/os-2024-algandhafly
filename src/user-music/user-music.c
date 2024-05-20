#include <stdint.h>
#include <stdbool.h>

void systemCall__music(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void play_music(uint32_t frequencies[], uint32_t sample_delay_ms, uint32_t length){
    for(uint32_t i = 0; i < length; i++){
        systemCall__music(30, frequencies[i], 0, 0);
        for(uint64_t j = 0; j < sample_delay_ms * 100000; j++);
    }
    systemCall__music(30, 0, 0, 0);
}

int main(void) {

    uint32_t bad_apple[] = {
587,// d
659,// e
698,// f
784,// g
880,// a
880,// a
1175,// d
1046,// c
880,// a
880,// a
587,// d
587,// d
880,// a
784,// g
698,// f
659,// e
587,// d
659,// e
698,// f
784,// g
880,// a
880,// a
784,// g
698,// f
659,// e
440,// a_
659,// e
698,// f
659,// e
587,// d
543,// c?
659,// e
    };
    uint8_t sample_delay_ms = 50;
    uint8_t length = 32;
    
    play_music(bad_apple, sample_delay_ms, length);

    return 0;
}
