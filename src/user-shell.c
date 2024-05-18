#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

uint32_t currentDirCluster = 0x2;
struct FAT32DirectoryTable currentDir;
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
struct SyscallPutsArgs {
    char* buf;
    uint32_t count;
    uint32_t fg_color;
    uint32_t bg_color;
};

struct StringN {
    char buf[256];
    uint32_t len;
};

struct charBuff {
    char* buf;
    uint32_t count;
};

uint32_t strlen(char* buf) {
    uint32_t count = 0;
    while (*buf != '\0') {
        count++;
        buf++;
    }
    return count;
}
void puts(char* buf, uint32_t color) {
    syscall(6, (uint32_t) buf, strlen(buf), color);
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

void cetak_prompt() {
    char* os1 = "Istiqomah@OS-IF2230:";
    char* os2 = "/";
    char* os3 = "$";
    syscall(6, (uint32_t)os1 , strlen(os1), 0x5);
    syscall(6, (uint32_t)os2 , strlen(os2), 0x6);
    syscall(6, (uint32_t)os3 , strlen(os3), 0x7);
}

// void set_current_cluster() {
//     struct FAT32DirectoryEntry curr_entry = currentDir.table[0];
//     currentDirCluster = (curr_entry.cluster_high << 16) | curr_entry.cluster_low;
// }

void mkdir(struct StringN folder_Name){
    struct FAT32DriverRequest request = {
        .name = "\0\0\0\0\0\0\0\0",
        .parent_cluster_number = currentDirCluster,
        .buffer_size = 0,
    };
    if(folder_Name.len > 8){
        syscall(6, (uint32_t) "Directory name is too long! (Maximum 8 Characters)", 51, 0xC);
    }
    else{
        for (uint8_t i = 0; i < folder_Name.len; i++) {
            request.name[i] = folder_Name.buf[i];
        }
        int8_t retcode;
        syscall(2, (uint32_t) &request, (uint32_t) &retcode, 0);
        switch (retcode) {
        case 0:
            syscall(6, (uint32_t)   "Operation success! " , 20, 0xE);
            syscall(6, (uint32_t) "'", 1, 0xE);
            syscall(6, (uint32_t) folder_Name.buf, strlen(folder_Name.buf), 0xE);
            syscall(6, (uint32_t) "'", 1, 0xE);
            syscall(6, (uint32_t) " has been created..\n", 20, 0xE);
            break;
        case 1:
            syscall(6, (uint32_t) "mkdir: cannot create directory ", 32, 0xC);
            syscall(6, (uint32_t) "'", 1, 0xC);

            syscall(6, (uint32_t) folder_Name.buf, strlen(folder_Name.buf), 0xC);
            syscall(6, (uint32_t) "'", 1, 0xC);
            
            syscall(6, (uint32_t) ": File exists", 13, 0xC);
            
            break;
        default:
            break;
        }
    }
}

void ls(){
    struct FAT32DirectoryTable table = currentDir;
    for (int i = 2; i < 64; i++) {
        // Check if entry is empty
        struct FAT32DirectoryEntry entry = table.table[i];
        puts(entry.name, 0x7);
        if (entry.name[0] == '\0') {
            break;
        }

        // Print entry name
        struct SyscallPutsArgs args = {
            .buf = entry.name,
            .count = strlen(args.buf),
            .fg_color = 0x7,
            .bg_color = 0x0
        };

        syscall(6, (uint32_t) &entry.name, strlen(entry.name), 0);

        // Print space between entries
        struct SyscallPutsArgs args2 = {
            .buf = " ",
            .count = strlen(args2.buf),
            .fg_color = 0x7,
            .bg_color = 0x0
        };
        syscall(6, (uint32_t) &args2, args2.count, 0);
    }
}

void parseCommand(struct StringN input){
    struct StringN perintah;
    stringn_create(&perintah);

    uint32_t i;
    for (i = 0; i < input.len; i++) {
        if (input.buf[i] == ' ') {
            break;
        }
        stringn_appendchar(&perintah, input.buf[i]);
    }

    struct StringN variabel;
    stringn_create(&variabel);
    i++;
    for (i = i; i < input.len; i++) {
        stringn_appendchar(&variabel, input.buf[i]);
    }

    if (memcmp(perintah.buf, "cd", 2) == 0)
    {
        syscall(6, (uint32_t) "\ncd\n", 5, 0x2);
        cetak_prompt();
    } 
    else if (memcmp(perintah.buf, "ls", 2) == 0)
    {
        syscall(6, (uint32_t) "\nls\n", 5, 0x2);
        ls();
        cetak_prompt();
    }
    else if (memcmp(perintah.buf, "mkdir", 5) == 0)
    {
        syscall(6, (uint32_t) "\nmkdir\n", 8, 0x2);

        syscall(6, (uint32_t) variabel.buf, (uint32_t) variabel.len, 0x2);
        syscall(6, (uint32_t) "\n", 2, 0x2);

        mkdir(variabel);

        cetak_prompt();
    }
    else
    {
        syscall(6, (uint32_t) "\nCommand not found\n", 20, 0x4);
        cetak_prompt();
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
    int32_t retcode = 0;
    syscall(1, (uint32_t) &request, (uint32_t) &retcode, 0);

    currentDirCluster = 2;
    // set_current_cluster();
    
    char buf;
    struct charBuff args = {
        .buf = &buf,
        .count = 1,
    };

    struct StringN input;
    stringn_create(&input);
    

    cetak_prompt();
    
    syscall(7, 0, 0, 0);
    while (true) {
        syscall(4, (uint32_t) &buf, 0, 0);
        if (buf == '\n') {
            parseCommand(input);
            
            stringn_create(&input);
        } else if (buf != '\0') {
            syscall(5, (uint32_t) &args, 2, 0x7);
            stringn_appendchar(&input, buf);
        }
    }

    return 0;
}
