#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

struct time_t {
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned int year;
} __attribute__((packed));
typedef struct time_t time_t;


void get_indonesian_time(time_t *time);

void refresh_screen_clock();

uint16_t date_to_byte(time_t* t);
uint16_t time_to_byte(time_t* t);
void to_time_t(uint16_t date, uint16_t time, time_t* t);
void print_time_t(time_t* t);

#endif
