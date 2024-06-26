#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/cpu/portio.h"


/**
 * Set cursor to specified location. Row and column starts from 0
 * 
 * @param r row
 * @param c column
*/
void framebuffer_set_cursor(uint8_t r, uint8_t c) {
    uint16_t cursor_index = r * 80 + c;
    out(CURSOR_PORT_CMD, 0x0F);
    out(CURSOR_PORT_DATA, (uint8_t) (cursor_index & 0xFF));
    out(CURSOR_PORT_CMD, 0x0E);
    out(CURSOR_PORT_DATA, (uint8_t) ((cursor_index >> 8) & 0xFF));
}


/**
 * Set framebuffer character and color with corresponding parameter values.
 * More details: https://en.wikipedia.org/wiki/BIOS_color_attributes
 *
 * @param row Vertical location (index start 0)
 * @param col Horizontal location (index start 0)
 * @param c   Character
 * @param fg  Foreground / Character color
 * @param bg  Background color
 */
void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg) {
    int frame_buffer_index = (row * 80 + col) * 2;
    FRAMEBUFFER_MEMORY_OFFSET[frame_buffer_index] = c;
    FRAMEBUFFER_MEMORY_OFFSET[frame_buffer_index + 1] = (bg << 4) | fg;   
}


/**
 * Set all cell in framebuffer character to 0x00 (empty character)
 * and color to 0x07 (gray character & black background)
 * Extra note: It's allowed to use different color palette for this
 *
 */
void framebuffer_clear(void) {
    size_t FRAMEBUFFER_MEMORY_OFFSET_size = 80 * 25 * 2;
    for (size_t i = 0; i < FRAMEBUFFER_MEMORY_OFFSET_size; i++)
        if (i % 2 == 0){
            FRAMEBUFFER_MEMORY_OFFSET[i] = 0x00;
        }
        else {
            FRAMEBUFFER_MEMORY_OFFSET[i] = 0x07;
        }
}