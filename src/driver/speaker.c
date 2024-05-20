#include "header/text/framebuffer.h"
#include "header/driver/speaker.h"
#include "header/cpu/portio.h"

//Play sound using built-in speaker
void play_sound(uint32_t nFrequence) {
    // framebuffer_write_int(23, 0, nFrequence, White, Black);

    uint32_t Div;
    uint8_t tmp;

    //Set the PIT to the desired frequency
    Div = 1193180 / nFrequence;
    out(0x43, 0xb6); // Squarewave
    out(0x42, (uint8_t) (Div & 0xFF) ); 
    out(0x42, (uint8_t) (Div >> 8));

    //And play the sound using the PC speaker
    tmp = in(0x61);
    if (tmp != (tmp | 3)) {
        out(0x61, tmp | 3);
    }
}


#define delay(d) for(uint64_t j = 0; j < d * 100000; j++);
#define b3 play_sound(500); delay(60);
#define c play_sound(533); delay(60);
#define d play_sound(597); delay(60);
#define e play_sound(659); delay(60);
#define f play_sound(698); delay(60);
#define g play_sound(784); delay(60);
#define a play_sound(880); delay(60);
#define b play_sound(992); delay(60);
#define c5 play_sound(1046); delay(60);
#define d5 play_sound(1175); delay(60);

void play_boot_music() {
    
    d5 c5 e e g g
    b a c c d d
    a g b3 b3 d d g g g g

    stop_sound();
    delay(240);
    
    c5
    stop_sound();
}

//make it shut up
void stop_sound() {
    uint8_t tmp = in(0x61) & 0xFC;
    out(0x61, tmp);
}
