#include "header/driver/keyboard.h"
#include "header/text/framebuffer.h"
#include "header/cpu/portio.h"
#include "header/stdlib/string.h"

static int row = 0;
static int col = 0;
static bool key_pressed = false;
static bool backspace_pressed = false;

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

struct KeyboardDriverState keyboard_state = {false, false, '\0'};

// Fungsi untuk mendapatkan nilai row
int get_row(void) {
    return row;
}

// Fungsi untuk mengubah nilai row
void set_row(int value) {
    row = value;
}

// Fungsi untuk mendapatkan nilai col
int get_col(void) {
    return col;
}

// Fungsi untuk mengubah nilai col
void set_col(int value) {
    col = value;
}

void keyboard_state_activate(void) {
    keyboard_state.keyboard_input_on = true;
    // Enable keyboard interrupt here (implementation dependent)
}

void keyboard_state_deactivate(void) {
    keyboard_state.keyboard_input_on = false;
    // Disable keyboard interrupt here (implementation dependent)
}

void get_keyboard_buffer(char *buf) {
    *buf = keyboard_state.keyboard_buffer;
    keyboard_state.keyboard_buffer = '\0';
}

int8_t terminal_length = 0;

void keyboard_isr(void) {
    if (!keyboard_state.keyboard_input_on) {
        keyboard_state.keyboard_buffer = '\0';
    } else {
        uint8_t scancode = in(KEYBOARD_DATA_PORT);
        char mapped_char = keyboard_scancode_1_to_ascii_map[scancode];
        if (mapped_char == '\b') {
            if (col >= terminal_length + 1) {
                backspace_pressed = true;
                framebuffer_write(row, col-1, '\0', 0x0F, 0x00);
                framebuffer_set_cursor(row, col-1);
                col -= 1;
                keyboard_state.keyboard_buffer = '\0';
            }
        } else if (scancode == 0x1C && !key_pressed) {
            keyboard_state_deactivate();
            row++;
            col = 0;
            framebuffer_set_cursor(row, col);
            key_pressed = true;
            keyboard_state.keyboard_buffer = '\n';
            keyboard_state_activate();
        } else if (scancode >= 0x02 && scancode <= 0x4A && !key_pressed) {
            framebuffer_write(row, col, mapped_char, 0x0F, 0x00);
            framebuffer_set_cursor(row, col + 1);
            keyboard_state.keyboard_buffer = mapped_char;
            key_pressed = true;
        } else if (scancode >= 0x80 && backspace_pressed) {
            backspace_pressed = false;
            if (keyboard_state.keyboard_buffer != '\0') {
                keyboard_state.keyboard_buffer = '\0';
                col--;
            }
        } else if (scancode >= 0x80 && scancode != 0x9C && key_pressed) {
            key_pressed = false;
            col++;
        } else if (scancode == 0x9C) {
            key_pressed = false;
        } else if (scancode == 0xe0) {
            col--;
            framebuffer_set_cursor(row, col);
        }
    }
    pic_ack(IRQ_KEYBOARD);
}
