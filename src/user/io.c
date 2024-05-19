#include <io.h>
#include <system.h>
#include <stddef.h>
#include <string.h>

void puts(char *str) {
    systemCall(5, (uint32_t) str, Color_White, Color_Black);
}
void puts_color(char *str, uint8_t fg, uint8_t bg) {
    systemCall(5, (uint32_t) str, fg, bg);
}

void put_char_color(char c, uint8_t fg, uint8_t bg){
    systemCall(5, (uint32_t) &c, fg, bg);
}

void puts_clamped(char *str, uint8_t max_length) {
    char str_plus_one[max_length+1];
    
    for(uint8_t i = 0; i < max_length; i++){
        str_plus_one[i] = str[i];
    }
    str_plus_one[max_length] = '\0';

    puts(str_plus_one);
}

void put_int_color(int num, uint8_t fg, uint8_t bg) {
    if(num == 0) {
        puts_color("0", fg, bg);
        return;
    }
    char str[32];
    size_t i = 0;
    bool is_negative = false;
    if(num < 0) {
        num = -num;
        is_negative = true;
    }
    while(num > 0) {
        str[i] = (char)(num % 10 + '0');
        num /= 10;
        i++;
    }
    
    char fliped_str[i+2];
    if(is_negative) {
        fliped_str[0] = '-';
        i++;
        for(size_t j = 1; j < i; j++) {
            fliped_str[j] = str[i-j-1];
        }
    } else {
        for(size_t j = 0; j < i; j++) {
            fliped_str[j] = str[i-j-1];
        }
    }
    fliped_str[i] = '\0';
    puts_color(fliped_str, fg, bg);
}

void put_int(int num) {
    put_int_color(num, Color_White, Color_Black);
}

void put_char(char c) {
    systemCall(5, (uint32_t) &c, 0, 0);
}

void gets(char *buf) {
    systemCall(6, (uint32_t) buf, 0, 0);
}


void get_line(char *buf) {
    memset(buf, 0, 4000);
    uint16_t t = 0;


    char temp[100];
    memset(temp, 0, sizeof(temp));

    struct {
        uint8_t cursor_x;
        uint8_t cursor_y;
    } __attribute__((packed)) cursor;
    systemCall(20, (uint32_t ) &cursor, 0, 0);

    const uint8_t CURSOR_X = cursor.cursor_x;
    const uint8_t CURSOR_Y = cursor.cursor_y;

    while (true) {
        systemCall(6, (uint32_t) temp, Color_White, Color_Black);

        if (temp[0] == '\0') {
            continue;
        }


        int i;
        for (i = 0; i < 100 && temp[i] != '\0'; i++) {
            if (temp[i] == '\b') {
                if (t > 0) {
                    t--;
                    buf[t] = '\0';
                }
            } else {
                buf[t++] = temp[i];
            }
        }
        systemCall(9, CURSOR_Y, CURSOR_X, 0);
        systemCall(19, 0, buf, 0);
        
        


        temp[0] = '\0';
        for (int i = 0; i < 4000 && buf[i] != '\0'; i++) {
            if (buf[i] == '\n') {
                buf[i] = '\0';                
                return;
            }
        }
    }
}
void get_line_color(char *buf, uint8_t fg, uint8_t bg) {
    systemCall(6, (uint32_t) buf, fg, bg);
}

void clear_screen() {
    systemCall(8, 0, 0, 0);
}
void set_cursor(uint8_t x, uint8_t y) {
    systemCall(9, x, y, 0);
}

