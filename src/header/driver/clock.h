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


void read_rtc(time_t *time);

#endif
