#include "gdt.h"

// Define the global_descriptor_table
struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        // Null Descriptor
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        // Kernel Code Segment Descriptor
        {0xFFFF, 0, 0, 0x9A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        // Kernel Data Segment Descriptor
        {0xFFFF, 0, 0, 0x92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    }
};

// Define the _gdt_gdtr
struct GDTR _gdt_gdtr = {
    .size = sizeof(global_descriptor_table.table) - 1,
    .address = &global_descriptor_table
};