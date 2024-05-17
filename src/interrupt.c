#include "header/cpu/interrupt.h"
#include "header/cpu/portio.h"
#include "header/cpu/idt.h"
#include "header/driver/keyboard.h"
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"

void io_wait(void) {
    out(0x80, 0);
}

void pic_ack(uint8_t irq) {
    if (irq >= 8) out(PIC2_COMMAND, PIC_ACK);
    out(PIC1_COMMAND, PIC_ACK);
}

void pic_remap(void) {
    // Starts the initialization sequence in cascade mode
    out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); 
    io_wait();
    out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC1_DATA, PIC1_OFFSET); // ICW2: Master PIC vector offset
    io_wait();
    out(PIC2_DATA, PIC2_OFFSET); // ICW2: Slave PIC vector offset
    io_wait();
    out(PIC1_DATA, 0b0100); // ICW3: tell Master PIC, slave PIC at IRQ2 (0000 0100)
    io_wait();
    out(PIC2_DATA, 0b0010); // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    out(PIC1_DATA, ICW4_8086);
    io_wait();
    out(PIC2_DATA, ICW4_8086);
    io_wait();

    // Disable all interrupts
    out(PIC1_DATA, PIC_DISABLE_ALL_MASK);
    out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}



void activate_keyboard_interrupt(void) {
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_KEYBOARD));
}

struct TSSEntry _interrupt_tss_entry = {
    .ss0  = GDT_KERNEL_DATA_SEGMENT_SELECTOR,
};

void set_tss_kernel_current_stack(void) {
    uint32_t stack_ptr;
    // Reading base stack frame instead esp
    __asm__ volatile ("mov %%ebp, %0": "=r"(stack_ptr) : /* <Empty> */);
    // Add 8 because 4 for ret address and other 4 is for stack_ptr variable
    _interrupt_tss_entry.esp0 = stack_ptr + 8; 
}

static uint8_t cursor_row = 0;
static uint8_t cursor_col = 0;

void update_cursor_position() {
    framebuffer_set_cursor(cursor_row, cursor_col);
}

void puts(const char* str, uint32_t length, uint32_t attribute) {
    uint8_t fg = attribute & 0x0F; // Ambil 4 bit pertama untuk foreground
    uint8_t bg = (attribute >> 4) & 0x0F; // Ambil 4 bit berikutnya untuk background

    for (uint32_t i = 0; i < length; i++) {
        if (str[i] == '\0') {
            break; // Berhenti jika mencapai akhir string
        }
        if (str[i] == '\n') {
            cursor_row++;
            cursor_col = 0;
            set_col(cursor_col);
            set_row(cursor_row);
        } else {
            framebuffer_write(cursor_row, cursor_col, str[i], fg, bg);
            cursor_col++;
            set_col(cursor_col);
            if (cursor_col >= 80) {
                cursor_col = 0;
                cursor_row++;
                set_col(cursor_col);
                set_row(cursor_row);
            }
        }
        if (cursor_row >= 25) {
            cursor_row = 0; // Menggulirkan layar jika perlu
            set_col(cursor_col);
        }
    }
    update_cursor_position();
}
uint8_t row_now = 0;
void puts_terminal(char *str, uint32_t len, uint32_t color)
{
    for (uint32_t i = 0; i < len; i++)
    {
        framebuffer_write(row_now, i, str[i], color, 0);
    }
}

void syscall(struct InterruptFrame frame) {
    switch (frame.cpu.general.eax) {
        case 0:
            *((int8_t*) frame.cpu.general.ecx) = read(
                *(struct FAT32DriverRequest*) frame.cpu.general.ebx
            );
            break;
        case 4:
            get_keyboard_buffer((char*) frame.cpu.general.ebx);
            break;
        case 6:
            puts(
                (char*) frame.cpu.general.ebx, 
                frame.cpu.general.ecx, 
                frame.cpu.general.edx
            ); // Assuming puts() exist in kernel
            break;
        case 7: 
            keyboard_state_activate();
            break;
    }
}

void main_interrupt_handler(struct InterruptFrame frame) {
    switch (frame.int_number) {
        case PIC1_OFFSET + IRQ_KEYBOARD:
            keyboard_isr();
            break;
        case (0x30):
            syscall(frame);
            break;
    }
}