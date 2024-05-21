#ifndef TIME_H
#define TIME_H

#include <stdint.h>
#include "header/driver/clock.h"

time_t now();

uint8_t get_second(time_t t);
uint8_t get_minute(time_t t);
uint8_t get_hour(time_t t);




#endif
