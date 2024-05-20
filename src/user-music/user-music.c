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

void int_to_char(int i, char* c){
    c[0] = i / 100 + '0';
    c[1] = (i % 100) / 10 + '0';
    c[2] = i % 10 + '0';
    c[3] = '\0';
}
void print4(char* c){
    systemCall__music(5, c[0], 15, 0);
    systemCall__music(5, c[1], 15, 0);
    systemCall__music(5, c[2], 15, 0);
    systemCall__music(5, c[3], 15, 0);
    systemCall__music(5, ' ', 15, 0);
}
void print_int(int i){
    char c[4];
    int_to_char(i, c);
    print4(c);
}

void play_music(uint32_t frequencies[], uint32_t sample_delay_ms, uint32_t length){
    for(uint32_t i = 0; i < length; i++){
        systemCall__music(30, frequencies[i], 0, 0);
        for(uint64_t j = 0; j < sample_delay_ms * 50000; j++);
    }
    systemCall__music(30, 0, 0, 0);
}

int main(void) {

    uint32_t bad_apple[] = {
// Bridge
587,// d#
587,// d#
523,// c#
587,// d#
587,// d#
523,// c#
587,// d#
587,// d#
523,// c#
587,// d#
587,// d#
784,// g#
784,// g#

587,// d#
587,// d#
523,// c#
587,// d#
587,// d#
523,// c#
587,// d#
587,// d#
523,// c#
587,// d#
587,// d#
784,// g#
784,// g#

587,// d#
587,// d#
523,// c#
587,// d#
587,// d#
523,// c#
587,// d#
587,// d#
523,// c#
587,// d#
587,// d#
784,// g#
784,// g#

587,// d#
587,// d#
523,// c#
587,// d#
587,// d#
523,// c#
587,// d#
587,// d#
523,// c#
587,// d#
587,// d#
784,// g#
784,// g#


// Verse
587,// d#
659,// e#
698,// f#
784,// g#
880,// a#
880,// a#
1175,// d#6
1046,// c#6

880,// a#
880,// a#
587,// d#
587,// d#
880,// a#
784,// g#
698,// f#
659,// e#

587,// d#
659,// e#
698,// f#
784,// g#
880,// a#
880,// a#
784,// g#
698,// f#

659,// e#
587,// d#
659,// e#
698,// f#
659,// e#
587,// d#
523,// c#_
659,// e#

587,// d#
659,// e#
698,// f#
784,// g#
880,// a#
880,// a#
1175,// d#6
1046,// c#6

880,// a#
880,// a#
587,// d#
587,// d#
880,// a#
784,// g#
698,// f#
659,// e#

587,// d#
659,// e#
698,// f#
784,// g#
880,// a#
880,// a#
784,// g#
698,// f#

659,// e#
659,// e#
698,// f#
698,// f#
784,// g#
784,// g#
880,// a#
880,// a#


// Chorus
1046,// c#6
1175,// d#6
880,// a#
784,// g#
880,// a#
880,// a#
784,// g#
880,// a#

1046,// c#6
1175,// d#6
880,// a#
784,// g#
880,// a#
880,// a#
784,// g#
880,// a#

784,// g#
698,// f#
659,// e#
523,// c#
587,// d#
587,// d#
523,// c#
587,// d#

659,// e#
698,// f#
784,// g#
880,// a#
587,// d#
587,// d#
880,// a#
1046,// c#6



// Chorus 2nd part
1046,// c#6
1175,// d#6
880,// a#
784,// g#
880,// a#
880,// a#
784,// g#
880,// a#

1046,// c#6
1175,// d#6
880,// a#
784,// g#
880,// a#
880,// a#
1174,// d#6
1318,// e#6

1397,// f#6
1318,// e#6
1175,// d#6
1046,// c#6
880,// a#
880,// a#
784,// g#
880,// a#

784,// g#
698,// f#
659,// e#
523,// c#4
587,// d#
587,// d#
587,// d#


    };
    uint8_t sample_delay_ms = 200;
    uint8_t length = 179;
    
    play_music(bad_apple, sample_delay_ms, length);

    return 0;
}
