#include "header/text/framebuffer.h"
#include "header/driver/speaker.h"
#include "header/cpu/portio.h"

//Play sound using built-in speaker
void play_sound(uint32_t nFrequence) {
    framebuffer_write_int(23, 0, nFrequence, White, Black);

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

//make it shut up
void stop_sound() {
    uint8_t tmp = in(0x61) & 0xFC;
    out(0x61, tmp);
}
