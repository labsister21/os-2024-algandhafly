#ifndef SPEAKER_H
#define SPEAKER_H
#include <stdint.h>

//Play sound using built-in speaker
void play_sound(uint32_t nFrequence);

//make it shut up
void stop_sound();

#endif