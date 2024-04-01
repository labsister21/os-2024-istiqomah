#include "gdt.h"

// Define the global_descriptor_table
struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        // Null Descriptor
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        // Kernel Code Segment Descriptor
        {0xFFFF, 0, 0, 0xA, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        // Kernel Data Segment Descriptor
        {0xFFFF, 0, 0, 0x2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        // ... You can add more descriptors here ...
    }
};

// Define the _gdt_gdtr
struct GDTR _gdt_gdtr = {
    .size = sizeof(global_descriptor_table.table) - 1,
    .address = &global_descriptor_table
};