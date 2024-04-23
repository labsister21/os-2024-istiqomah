#ifndef _PAGING_H
#define _PAGING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define SYSTEM_MEMORY_MB     128
#define PAGE_ENTRY_COUNT     1024
#define PAGE_FRAME_SIZE      (1 << (2 + 10 + 10))
#define PAGE_FRAME_MAX_COUNT ((SYSTEM_MEMORY_MB << 20) / PAGE_FRAME_SIZE)

extern struct PageDirectory _paging_kernel_page_directory;

struct PageDirectoryEntryFlag {
    uint8_t present_bit        : 1;
    uint8_t write_bit          : 1;
    uint8_t user_bit           : 1;
    uint8_t write_through_bit  : 1;
    uint8_t cache_disabled_bit : 1;
    uint8_t accessed_bit       : 1;
    uint8_t dirty_bit          : 1;
    uint8_t page_size_bit      : 1;
} __attribute__((packed));

struct PageDirectoryEntry {
    struct PageDirectoryEntryFlag flag;
    uint16_t lower_address   : 10;
    uint16_t reserved_1      : 1;
    uint16_t available       : 3;
    uint16_t higher_address  : 8;
} __attribute__((packed));

struct PageDirectory {
    struct PageDirectoryEntry table[PAGE_ENTRY_COUNT];
} __attribute__((aligned(0x1000)));

struct PageManagerState {
    bool     page_frame_map[PAGE_FRAME_MAX_COUNT];
    uint32_t free_page_frame_count;
} __attribute__((packed));

void update_page_directory_entry(
    struct PageDirectory *page_dir,
    void *physical_addr, 
    void *virtual_addr, 
    struct PageDirectoryEntryFlag flag
);

void flush_single_tlb(void *virtual_addr);

bool paging_allocate_check(uint32_t amount);

bool paging_allocate_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr);

bool paging_free_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr);

#endif