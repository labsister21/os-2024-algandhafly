#include "header/driver/keyboard.h"
#include "header/text/framebuffer.h"
#include "header/cpu/portio.h"
#include "header/stdlib/string.h"

const char keyboard_scancode_1_to_ascii_map[256] = {
    0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',  // 0-15
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']', '\n',   0,  'a',  's',  // 16-31
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',  'z', 'x',  'c',  'v',  // 32-47
    'b',  'n', 'm', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,  // 48-63
    0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,  // 64-79
    0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,  // 80-95
    0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,  // 96-111
    0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,  // 112-127

    // Shifted states start here
    0, 0x1B, '!', '@', '#', '$', '%', '^',  '&', '*', '(',  ')',  '_', '+', '\b', '\t',  // 128-143
    'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',  'O', 'P', '{',  '}', '\n',   0,  'A',  'S',  // 144-159
    'D',  'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',   0,  '|',  'Z', 'X',  'C',  'V',  // 160-175
    'B',  'N', 'M', '<', '>', '?',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,  // 176-191
    0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '_',    0,    0,   0,  '+',    0,  // 192-207
    0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,  // 208-223
    0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,  // 224-239
    0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,  // 240-255
};


struct KeyboardDriverState keyboard_state = {
    .read_extended_mode = false,
    .keyboard_input_on = false,
    .keyboard_buffer = 0,
    .caps_lock_on = false,
};

// Activate keyboard ISR / start listen keyboard & save to buffer
void keyboard_state_activate(void){
    keyboard_state.keyboard_input_on = true;
}

// Deactivate keyboard ISR / stop listening keyboard interrupt
void keyboard_state_deactivate(void){
    keyboard_state.keyboard_input_on = false;
}

// Get keyboard buffer value and flush the buffer - @param buf Pointer to char buffer
void get_keyboard_buffer(char *buf){
    *buf = keyboard_state.keyboard_buffer;
    keyboard_state.keyboard_buffer = 0;
}

/* -- Keyboard Interrupt Service Routine -- */

/**
 * Handling keyboard interrupt & process scancodes into ASCII character.
 * Will start listen and process keyboard scancode if keyboard_input_on.
 */
void keyboard_isr(void){
    uint8_t scan_code = in(KEYBOARD_DATA_PORT);

    // Ignore "key up" scancodes
    if (scan_code & 0x80) {
        pic_ack(IRQ_KEYBOARD);
        return;
    }

    if (scan_code == 0x3A) {
        keyboard_state.caps_lock_on = !keyboard_state.caps_lock_on;
        pic_ack(IRQ_KEYBOARD);
        return;
    }

    if(keyboard_state.keyboard_input_on){
        int key_down_code = scan_code % (0b10000000);

        // handle shift pressing
        if (key_down_code == 0x2A) {
            keyboard_state.was_shift = true;
            pic_ack(IRQ_KEYBOARD);
            return;
        } else if (key_down_code == 0x2A + 0b10000000) {
            keyboard_state.was_shift = false;
            pic_ack(IRQ_KEYBOARD);
            return;
        }


        if (key_down_code <= 55 && key_down_code >= 30) {
            scan_code = keyboard_state.caps_lock_on ? ((scan_code + 0b10000000) % 0b100000000) : scan_code;
        }

        // handle pressing shift with other key
        if (keyboard_state.was_shift) {
            scan_code = (scan_code + 128)%256;
        }

        char c = keyboard_scancode_1_to_ascii_map[scan_code];

        keyboard_state.keyboard_buffer = c;


        /**
         * Debugging purpose
        */
        // int col = 0;
        // framebuffer_clear();
        // while (scan_code > 0) {
        //     framebuffer_write(20, col++, (scan_code % 10) + '0', 0xF, 0);
        //     scan_code /= 10;
        // }


        // Handle shift
        // if(keyboard_state.was_shift && c >= 'a' && c <= 'z') c -= 0x20;
        // if(scan_code == 0x2A) keyboard_state.was_shift = true;
        // else if(scan_code == 0x2A + 0b10000000){
        //   keyboard_state.was_shift = false;
        //   pic_ack(IRQ_KEYBOARD);
        //   return;
        // }   
        
    }
    pic_ack(IRQ_KEYBOARD);
}