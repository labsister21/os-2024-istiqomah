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
    // Calculate the offset based on the row and column
    #define FRAMEBUFFER_COLS 80 // Assuming 80 columns in the framebuffer

    uint16_t offset = (r * FRAMEBUFFER_COLS + c) * 2;

    // Set the cursor position by sending the high byte of the offset to port 0x3D4
    port_byte_out(0x3D4, 0x0F);
    port_byte_out(0x3D5, (uint8_t)(offset >> 8));

    // Set the cursor position by sending the low byte of the offset to port 0x3D4
    port_byte_out(0x3D4, 0x0E);
    port_byte_out(0x3D5, (uint8_t)offset);
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
    // Calculate the offset of the character in the framebuffer
    #define FRAMEBUFFER_WIDTH 25 // Assuming 80 columns in the framebuffer

    uint16_t offset = (row * FRAMEBUFFER_WIDTH + col) * 2;

    // Set the character at the specified location
    framebuffer[offset] = c;
    
    // Set the color attributes at the specified location
    framebuffer[offset + 1] = (bg << 4) | (fg & 0x0F);
}

/**
 * Set all cell in framebuffer character to 0x00 (empty character)
 * and color to 0x07 (gray character & black background)
 * Extra note: It's allowed to use different color palette for this
 *
 */
void framebuffer_clear(void) {
    memset(framebuffer, 0x00, FRAMEBUFFER_SIZE);
    memset(framebuffer + 1, 0x07, FRAMEBUFFER_SIZE - 1);
}
