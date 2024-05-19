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
    .show_on_screen = false,
    .keyboard_buffer = 0,
    .caps_lock_on = false,
    .command_state = {
        .command = {0},
        .command_length = 0,
        .cursor_at = 0,
    }
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


/**
 * Cursor
*/
void cursor_move_left() {
    if (keyboard_state.command_state.cursor_at == 0) return;

    if (framebuffer_state.cursor_x == 0 && framebuffer_state.cursor_y == 0) return;

    if (framebuffer_state.cursor_x == 0 && framebuffer_state.cursor_y > 0) {
        framebuffer_state.cursor_y--;
        framebuffer_state.cursor_x = 79;
    } else {
        framebuffer_state.cursor_x--;
    }

    keyboard_state.command_state.cursor_at--;
    framebuffer_set_cursor(framebuffer_state.cursor_y, framebuffer_state.cursor_x);
}

void cursor_move_right() {
    if (keyboard_state.command_state.cursor_at == keyboard_state.command_state.command_length) return;

    if (framebuffer_state.cursor_x == 79 && framebuffer_state.cursor_y == 24) return;

    if (framebuffer_state.cursor_x == 79 && framebuffer_state.cursor_y < 24) {
        framebuffer_state.cursor_y++;
        framebuffer_state.cursor_x = 0;
    } else {
        framebuffer_state.cursor_x++;
    }

    keyboard_state.command_state.cursor_at++;
    framebuffer_set_cursor(framebuffer_state.cursor_y, framebuffer_state.cursor_x);
}

void remove_at_before_cursor() {
    if (keyboard_state.command_state.cursor_at == 0) return;

    for (int i = keyboard_state.command_state.cursor_at - 1; i < keyboard_state.command_state.command_length - 1; i++) {
        keyboard_state.command_state.command[i] = keyboard_state.command_state.command[i + 1];
    }

    keyboard_state.command_state.command_length--;
    keyboard_state.command_state.cursor_at--;
    decrement(&framebuffer_state);
    framebuffer_set_cursor(framebuffer_state.cursor_y, framebuffer_state.cursor_x);
}

void insert_at_cursor() {
    if (keyboard_state.command_state.command_length == MAX_COMMAND_LENGTH) return;

    for (int i = keyboard_state.command_state.command_length; i > keyboard_state.command_state.cursor_at; i--) {
        keyboard_state.command_state.command[i] = keyboard_state.command_state.command[i - 1];
    }

    keyboard_state.command_state.command_length++;
    keyboard_state.command_state.command[keyboard_state.command_state.cursor_at] = keyboard_state.keyboard_buffer;

    if (keyboard_state.keyboard_buffer == '\n' || keyboard_state.keyboard_buffer == '\b') {
        return;
    }
    increment(&framebuffer_state);
    keyboard_state.command_state.cursor_at++;
    framebuffer_set_cursor(framebuffer_state.cursor_y, framebuffer_state.cursor_x);
}


/* -- Keyboard Interrupt Service Routine -- */

/**
 * Handling keyboard interrupt & process scancodes into ASCII character.
 * Will start listen and process keyboard scancode if keyboard_input_on.
 */
void keyboard_isr(void){
    uint8_t scan_code = in(KEYBOARD_DATA_PORT);



    // handle shift pressing
    if (scan_code == 0x2A || scan_code == 0x36) {
        keyboard_state.was_shift = true;
        pic_ack(IRQ_KEYBOARD);
        return;

    } else if (scan_code == 0x2A + 0b10000000 || scan_code == 0x36 + 0b10000000) {
        keyboard_state.was_shift = false;
        pic_ack(IRQ_KEYBOARD);
        return;
    }

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

    // Handle left and right arrow
    if (scan_code == 0x4B) {
        // cursor_move_left();
        pic_ack(IRQ_KEYBOARD);
        return;
    } else if (scan_code == 0x4D) {
        // cursor_move_right();
        pic_ack(IRQ_KEYBOARD);
        return;
    }

    if (keyboard_scancode_1_to_ascii_map[scan_code] == 0) {
        pic_ack(IRQ_KEYBOARD);
        return;
    }



    if(keyboard_state.keyboard_input_on){
        int key_down_code = scan_code % (0b10000000);
        
        if ((key_down_code <= 25 && key_down_code >= 16) || (key_down_code <= 38 && key_down_code >= 30) || (key_down_code <= 50 && key_down_code >= 44)) {
            scan_code = keyboard_state.caps_lock_on ? ((scan_code + 0b10000000) % 0b100000000) : scan_code;
        }

        // handle pressing shift with other key
        if (keyboard_state.was_shift) {
            scan_code = (scan_code + 128) % 256;
        }

        char c = keyboard_scancode_1_to_ascii_map[scan_code];

        keyboard_state.keyboard_buffer = c;   
        if (c == '\n') {
            insert_at_cursor();
            // framebuffer_write_and_move_cursor_until_null(keyboard_state.command_state.command, White, Black);
            pic_ack(IRQ_KEYBOARD);
            return;
        }

        if (keyboard_state.keyboard_buffer == '\b') {
            insert_at_cursor();
        } else if (keyboard_state.command_state.command_length < MAX_COMMAND_LENGTH) {
            insert_at_cursor();
        }

    }
    pic_ack(IRQ_KEYBOARD);
}

void get_current_cursor(struct FramebufferState *state) {
    state->cursor_x = framebuffer_state.cursor_x;
    state->cursor_y = framebuffer_state.cursor_y;
}

void update_text(char c) {
    if (c == '\b') {
        // Handles backspace

        for (int i = 0; i < keyboard_state.command_state.command_length - keyboard_state.command_state.cursor_at; i++) {
            framebuffer_write(framebuffer_state.cursor_y, framebuffer_state.cursor_x + i, keyboard_state.command_state.command[keyboard_state.command_state.cursor_at + i], White, Black);
        }

        framebuffer_write(framebuffer_state.cursor_y, framebuffer_state.cursor_x + keyboard_state.command_state.command_length - keyboard_state.command_state.cursor_at, ' ', White, Black);
    } else if (c) {
        for (int i = 1; i <= keyboard_state.command_state.command_length - keyboard_state.command_state.cursor_at + 1; i++) {
            framebuffer_write(framebuffer_state.cursor_y, 
        framebuffer_state.cursor_x + keyboard_state.command_state.command_length - keyboard_state.command_state.cursor_at - i,
        keyboard_state.command_state.command[keyboard_state.command_state.command_length - i], White, Black);
        }

    }
}

void get_command_buffer(char *buf) {
    memcpy(buf, keyboard_state.command_state.command, keyboard_state.command_state.command_length);
    clear_command_buffer();
}

void clear_command_buffer() {
    keyboard_state.command_state.command_length = 0;
    for (int i = 0; i < MAX_COMMAND_LENGTH; i++) {
        keyboard_state.command_state.command[i] = '\0';
    }
    kernel_puts(keyboard_state.command_state.command, White, Black);

    keyboard_state.keyboard_buffer = 0;
    keyboard_state.command_state.cursor_at = 0;
}