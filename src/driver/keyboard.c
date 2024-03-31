#include "header/driver/keyboard.h"
#include "header/text/framebuffer.h"
#include "header/cpu/portio.h"
#include "header/stdlib/string.h"

const char keyboard_scancode_1_to_ascii_map[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
      'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']', '\n',   0,  'a',  's',
      'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',  'z', 'x',  'c',  'v',
      'b',  'n', 'm', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

struct KeyboardDriverState keyboard_state = {
    .read_extended_mode = false,
    .keyboard_input_on = false,
    .keyboard_buffer = 0
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

    if(keyboard_state.keyboard_input_on){
        char c = keyboard_scancode_1_to_ascii_map[scan_code];
        
        // Handle shift
        if(keyboard_state.was_shift && c >= 'a' && c <= 'z') c -= 0x20;
        if(scan_code == 0x2A) keyboard_state.was_shift = true;
        else if(scan_code == 0x2A + 0b10000000){
          keyboard_state.was_shift = false;
          pic_ack(IRQ_KEYBOARD);
          return;
        }
        
        keyboard_state.keyboard_buffer = c;
    }
    pic_ack(IRQ_KEYBOARD);
}