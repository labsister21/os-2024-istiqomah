#include <stdint.h>
#include "header/cpu/gdt.h"
#include "header/kernel-entrypoint.h"
#include "header/text/framebuffer.h"
#include "header/cpu/idt.h"
#include "header/cpu/interrupt.h"
#include "header/driver/keyboard.h"
#include "header/driver/disk.h"
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

// void kernel_setup(void) {
//     load_gdt(&_gdt_gdtr);
//     pic_remap();
//     initialize_idt();
//     activate_keyboard_interrupt();
//     framebuffer_clear();
//     framebuffer_set_cursor(0, 0);

//     //int col = 0;
//     keyboard_state_activate();
//     while (true) {
//         char c;
//         get_keyboard_buffer(&c);
//         //if (c) framebuffer_write(0, col++, c, 0xF, 0);
//     }
// }

// void kernel_setup(void) {
//     load_gdt(&_gdt_gdtr);
//     pic_remap();
//     activate_keyboard_interrupt();
//     initialize_idt();
//     framebuffer_clear();
//     framebuffer_set_cursor(0, 0);

//     write_blocks(fs_signature, 0, 1);

//     struct BlockBuffer b;
//     for (int i = 0; i < 512; i++) b.buf[i] = i % 16;
//     write_blocks(&b, 17, 1);
//     while (true);
// }

/* CRUD KERNEL */

void kernel_setup(void)
{
    load_gdt(&_gdt_gdtr);
    pic_remap();
    activate_keyboard_interrupt();
    initialize_idt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    initialize_filesystem_fat32();

    struct ClusterBuffer cbuf[5];
    for (uint32_t i = 0; i < 5; i++)
        for (uint32_t j = 0; j < CLUSTER_SIZE; j++)
            cbuf[i].buf[j] = i + 'a';

    struct FAT32DriverRequest request = {
        .buf = cbuf,
        .name = "root",
        .ext = "uwu",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size = 0,
    };

    write(request); 
    memcpy(request.name, "folder1\0\0\0", 8);
    write(request);

    memcpy(request.name, "daijobu", 8);
    memcpy(request.ext, "\0\0\0", 3);
    write(request); 

    memcpy(request.name, "folder2", 8);
    write(request); 

    memcpy(request.name, "kano\0\0\0\0\0", 8);
    write(request); 

    memcpy(request.name, "nbuba", 8);
    write(request); 
    // delete (request);
    while (true)
    {
        keyboard_state_activate();
    }
}
