#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}


// Helper structs
struct StringN {
    char buf[256];
    uint32_t len;
};

struct StringNList_Node {
    struct StringN data;
    struct StringNList_Node* next;
};

struct StringNList {
    struct StringNList_Node* head;
    struct StringNList_Node* tail;
};

// Helper functions
uint32_t strlen(char* buf) {
    uint32_t count = 0;
    while (*buf != '\0') {
        count++;
        buf++;
    }
    return count;
}

bool strcmp(char* str1, char* str2) {
    // Get lengths of str1 and str2
    uint32_t len1 = strlen(str1);
    uint32_t len2 = strlen(str2);

    // If lengths are not equal, return false
    if (len1 != len2) return false;
    
    // Compare each character
    while (*str1) {
        if (*str1 != *str2) return false;
        str1++;
        str2++;
    }
    
    return true;
}

void strcpy(char* dest, char* src) {
    while (*src) {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

// StringN operations
void stringn_create(struct StringN* str) {
    memset(str->buf, '\0', 256);
    str->len = 0;
}

void stringn_appendchar(struct StringN* str, char c) {
    str->buf[str->len] = c;
    str->len++;
}

void stringn_appendstr(struct StringN* str, char* buf) {
    while (*buf) {
        str->buf[str->len] = *buf;
        str->len++;
        buf++;
    }
}

int main(void) {
    struct ClusterBuffer      cl[2]   = {0};
    struct FAT32DriverRequest request = {
        .buf                   = &cl,
        .name                  = "shell",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = CLUSTER_SIZE,
    };
    int32_t retcode;
    syscall(0, (uint32_t) &request, (uint32_t) &retcode, 0);
    
    char buf[80];
    char* os = "Istiqomah@OS-IF2230";

    syscall(6, (uint32_t)os , strlen(os), 0x3);
    while (true) {
        syscall(7, 0, 0, 0);
        syscall(4, (uint32_t) &buf, 0, 0);
        syscall(5, (uint32_t) &buf, 0xF, 0);
    }

    return 0;
}

