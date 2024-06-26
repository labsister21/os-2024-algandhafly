#include "header/driver/clock.h"
#include "header/text/framebuffer.h"
#include "header/cpu/portio.h"

#define CURRENT_YEAR        2024                            // Change this each year!
 
int century_register = 0x00;                                // Set by ACPI table parsing code if possible
 
unsigned char second;
unsigned char minute;
unsigned char hour;
unsigned char day;
unsigned char month;
unsigned int year;
 
 
#define cmos_address 0x70
#define cmos_data    0x71
#define INDONESIAN_HOUR_OFFSET 7
 
 
int get_update_in_progress_flag() {
    out(cmos_address, 0x0A);
    return (in(cmos_data) & 0x80);
}
 
unsigned char get_RTC_register(int reg) {
    out(cmos_address, reg);
    return in(cmos_data);
}

 
void read_rtc(time_t *time) {
    unsigned char century;
    unsigned char last_second;
    unsigned char last_minute;
    unsigned char last_hour;
    unsigned char last_day;
    unsigned char last_month;
    unsigned char last_year;
    unsigned char last_century;
    unsigned char registerB;

    while (get_update_in_progress_flag());
    second = get_RTC_register(0x00);
    minute = get_RTC_register(0x02);
    hour = get_RTC_register(0x04);
    day = get_RTC_register(0x07);
    month = get_RTC_register(0x08);
    year = get_RTC_register(0x09);
    if(century_register != 0) {
        century = get_RTC_register(century_register);
    }

    do {
        last_second = second;
        last_minute = minute;
        last_hour = hour;
        last_day = day;
        last_month = month;
        last_year = year;
        last_century = century;

        while (get_update_in_progress_flag());
        second = get_RTC_register(0x00);
        minute = get_RTC_register(0x02);
        hour = get_RTC_register(0x04);
        day = get_RTC_register(0x07);
        month = get_RTC_register(0x08);
        year = get_RTC_register(0x09);
        if(century_register != 0) {
                century = get_RTC_register(century_register);
        }
    } while( (last_second != second) || (last_minute != minute) || (last_hour != hour) ||
            (last_day != day) || (last_month != month) || (last_year != year) ||
            (last_century != century) );

    registerB = get_RTC_register(0x0B);


    if (!(registerB & 0x04)) {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
        day = (day & 0x0F) + ((day / 16) * 10);
        month = (month & 0x0F) + ((month / 16) * 10);
        year = (year & 0x0F) + ((year / 16) * 10);
        if(century_register != 0) {
                century = (century & 0x0F) + ((century / 16) * 10);
        }
    }

    if (!(registerB & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    if(century_register != 0) {
        year += century * 100;
    } else {
        year += (CURRENT_YEAR / 100) * 100;
        if(year < CURRENT_YEAR) year += 100;
    }


    time->year = year;
    time->month = month;
    time->day = day;
    time->hour = hour;
    time->minute = minute;
    time->second = second;
}


void get_indonesian_time(time_t *time) {
    read_rtc(time);
    time->hour += INDONESIAN_HOUR_OFFSET;
    if(time->hour >= 24) {
        time->hour -= 24;
        time->day++;
    }
}



void write_int_offset_one_if_less_than_9(uint8_t row, uint8_t col, int num, uint8_t fg, uint8_t bg){
    if(num < 10){
        framebuffer_write(row, col, '0', fg, bg);
        framebuffer_write_int(row, col+1, num, fg, bg);
    } else {
        framebuffer_write_int(row, col, num, fg, bg);
    }
}

void update_clock_in_screen(time_t *time){
    // 00:00:00
    // 76543210
    write_int_offset_one_if_less_than_9(HEIGHT-1, WIDTH-8, time->hour, White, Black);
    framebuffer_write(HEIGHT-1, WIDTH-6, ':', White, Black);
    write_int_offset_one_if_less_than_9(HEIGHT-1, WIDTH-5, time->minute, White, Black);
    framebuffer_write(HEIGHT-1, WIDTH-3, ':', White, Black);
    write_int_offset_one_if_less_than_9(HEIGHT-1, WIDTH-2, time->second, White, Black);

    // 00/00/0000
    // 9876543210
    write_int_offset_one_if_less_than_9(HEIGHT-2, WIDTH-10, time->day, White, Black);
    framebuffer_write(HEIGHT-2, WIDTH-8, '/', White, Black);
    write_int_offset_one_if_less_than_9(HEIGHT-2, WIDTH-7, time->month, White, Black);
    framebuffer_write(HEIGHT-2, WIDTH-5, '/', White, Black);
    write_int_offset_one_if_less_than_9(HEIGHT-2, WIDTH-4, time->year, White, Black);
}


void refresh_screen_clock(){

    time_t current_time;
    get_indonesian_time(&current_time);

    update_clock_in_screen(&current_time);
}

// Convert time_t to uint16_t
// year 0 - 4095
// month 1 - 12
// day 1 - 31
uint16_t date_to_byte(time_t* t){
    uint16_t b = 0;
    b |= t->year << 9;
    b |= t->month << 5;
    b |= t->day;
    return b;
}

// hour 0 - 23
// minute 0 - 59
// second 0 - 59
uint16_t time_to_byte(time_t* t){
    uint16_t b = 0;
    b |= t->hour << 11;
    b |= t->minute << 5;
    b |= t->second;
    return b;
}


void to_time_t(uint16_t date, uint16_t time, time_t* t){
    t->year = date >> 9;
    t->month = (date >> 5) & 0xF;
    t->day = date & 0x1F;

    t->hour = time >> 11;
    t->minute = (time >> 5) & 0x3F;
    t->second = time & 0x1F;
}

void print_time_t(time_t* t){
    uint32_t x = framebuffer_state.cursor_x;
    uint32_t y = framebuffer_state.cursor_y;

    framebuffer_write_int(y, x, t->year, White, Black);
    framebuffer_write(y, x+4, '/', White, Black);
    framebuffer_write_int(y, x+5, t->month, White, Black);
    framebuffer_write(y, x+7, '/', White, Black);
    framebuffer_write_int(y, x+8, t->day, White, Black);
    framebuffer_write(y, x+10, ' ', White, Black);

    framebuffer_write_int(y, x+11, t->hour, White, Black);
    framebuffer_write(y, x+13, ':', White, Black);
    framebuffer_write_int(y, x+14, t->minute, White, Black);
    framebuffer_write(y, x+16, ':', White, Black);
    framebuffer_write_int(y, x+17, t->second, White, Black);


}