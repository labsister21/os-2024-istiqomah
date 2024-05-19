#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

uint32_t currentDirCluster;
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

void set_current_cluster() {
    struct FAT32DirectoryEntry curr_entry = currentDir.table[0];
    currentDirCluster = (curr_entry.cluster_high << 16) | curr_entry.cluster_low;
}

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
    puts(os1, 0x5);
    puts(os2, 0x6);
    puts(os3, 0x7);
}

void mkdir(struct StringN folder_Name){
    struct FAT32DriverRequest request = {
        .name = "\0\0\0\0\0\0\0\0",
        .parent_cluster_number = currentDirCluster,
        .buffer_size = 0,
    };
    if(folder_Name.len > 8){
        puts("Directory name is too long! (Maximum 8 Characters)", 0xC);
    }
    else{
        for (uint8_t i = 0; i < folder_Name.len; i++) {
            request.name[i] = folder_Name.buf[i];
        }
        int8_t retcode;
        syscall(2, (uint32_t) &request, (uint32_t) &retcode, 0);
        switch (retcode) {
        case 0:
            puts("Operation success! " , 0xE);
            puts("'", 0xE);
            puts(folder_Name.buf, 0xE);
            puts("'", 0xE);
            puts(" has been created..\n", 0xE);

            break;
        case 1:
            puts("mkdir: cannot create directory ", 0xC);
            puts("'", 0xC);

            puts(folder_Name.buf, 0xC);
            puts("'", 0xC);
            
            puts(": File exists", 0xC);
            
            break;
        default:
            break;
        }
    }
}

void rm(struct StringN folder){
    struct FAT32DriverRequest request = {
        .name = "\0\0\0\0\0\0\0\0",
        .parent_cluster_number = currentDirCluster,
        .buffer_size = 0,
    };
    struct SyscallPutsArgs args = {
        .buf = "Directory ",
        .count = strlen(args.buf),
        .fg_color = 0xC,
        .bg_color = 0x0
    };
    if(folder.len > 8){
        args.buf = "rm: cannot remove : name is too long! (Maximum 8 Characters)";
        args.count = strlen(args.buf);
        args.fg_color = 0xC;
        puts(args.buf, 0x2);
    }
    else{
       for (uint8_t i = 0; i < folder.len; i++) {
            request.name[i] = folder.buf[i];
        }
        int8_t retcode;
        syscall(8,(uint32_t) &request, (uint32_t) &retcode, 0);
        switch (retcode){
            case 0:
                args.buf = "Operation success! ";
                args.count = strlen(args.buf);
                args.fg_color = 0xE;
                puts(args.buf, 0x2);
                args.buf = "'";
                args.count = strlen(args.buf);
                args.fg_color = 0xE;
                puts(args.buf, 0x2);
                args.buf = folder.buf;
                args.count = strlen(args.buf);
                args.fg_color = 0xE;
                puts(args.buf, 0x2);
                args.buf = "'";
                args.count = strlen(args.buf);
                args.fg_color = 0xE;
                puts(args.buf, 0x2);
                args.buf = "has been removed..";
                args.count = strlen(args.buf);
                args.fg_color = 0xE;
                puts(args.buf, 0x2);
                break;
            case 1:
                args.buf = "rm: cannot remove '";
                args.count = strlen(args.buf);
                args.fg_color = 0xC;
                puts(args.buf, 0x2);
                args.buf = folder.buf;
                args.count = strlen(args.buf);
                args.fg_color = 0xC;
                puts(args.buf, 0x2);
                args.buf = "': No such file or directory";
                args.count = strlen(args.buf);
                args.fg_color = 0xC;
                puts(args.buf, 0x2);
                break;
        }
    }
}

void ls() {
    syscall(3, currentDirCluster, (uint32_t) &currentDir, 1);
    for (unsigned int i = 2; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++) {
        struct FAT32DirectoryEntry entry = currentDir.table[i];

        if (entry.name[0] == '\0') {
            continue;
        }

        puts(entry.name, 0x7);

        puts(" ", 0x7);
    }
    puts("\n", 0x7);
}

void cp (struct StringN filename) {
    // memcopy file lalu paste didirectory yang sama
    uint8_t buf[10 * CLUSTER_SIZE];

    struct FAT32DriverRequest request = {
        .buf = &buf,
        .name = "\0\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
        .parent_cluster_number = currentDirCluster,
        .buffer_size = 10 * CLUSTER_SIZE
    };

    for (uint8_t i = 0; i < filename.len; i++) {
        request.name[i] = filename.buf[i];
    }

    int8_t retcode;
    syscall(0, (uint32_t) &request, (uint32_t) &retcode, 0);

    struct FAT32DriverRequest copy = {
        .buf = (uint8_t*) request.buf,
        .name = "copy",
        .ext = "\0\0\0",
        .parent_cluster_number = currentDirCluster,
        .buffer_size = strlen(copy.buf),
    };
    syscall(2, (uint32_t) &copy, (uint32_t)&retcode, 0);
    switch (retcode) {
        case 0:
            puts(copy.buf, 0x2);
            break;
        case 1:
            puts("cat: ",0xC);
            puts(filename.buf, 0xC);
            puts(": Is a directory", 0xC);
            break;
        case 2:
            puts("cat: ", 0xC);
            puts(filename.buf, 0xC);
            puts(": No such file or directory", 0xC);
            break;
        default:
            break;
    }
    puts("\n", 0x7);
}

void cat(struct StringN filename) {
    uint8_t buf[10 * CLUSTER_SIZE];

    struct FAT32DriverRequest request = {
        .buf = &buf,
        .name = "\0\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
        .parent_cluster_number = currentDirCluster,
        .buffer_size = 10 * CLUSTER_SIZE
    };

    for (uint8_t i = 0; i < filename.len; i++) {
        request.name[i] = filename.buf[i];
    }

    int8_t retcode;
    syscall(0, (uint32_t) &request, (uint32_t) &retcode, 0);

    switch (retcode) {
        case 0:
            puts(request.buf, 0x2);
            break;
        case 1:
            puts("cat: ",0xC);
            puts(filename.buf, 0xC);
            puts(": Is a directory", 0xC);
            break;
        case 2:
            puts("cat: ", 0xC);
            puts(filename.buf, 0xC);
            puts(": No such file or directory", 0xC);
            break;
        default:
            break;
    }
    puts("\n", 0x7);
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
        cetak_prompt();
    } 
    else if (memcmp(perintah.buf, "ls", 2) == 0)
    {
        ls();
        cetak_prompt();
    }
    else if (memcmp(perintah.buf, "mkdir", 5) == 0)
    {
        mkdir(variabel);
        cetak_prompt();
    }
    else if (memcmp(perintah.buf, "cat", 3) == 0)
    {
        cat(variabel);
        cetak_prompt();
    }
    else if (memcmp(perintah.buf, "rm", 2) == 0)
    {
        rm(variabel);
        cetak_prompt();
    }
    else if (memcmp(perintah.buf, "cp", 2) == 0)
    {
        cp(variabel);
        cetak_prompt();
    }
    else
    {
        puts("\nCommand not found\n", 0x4);
        cetak_prompt();
    }
}

int main(void) {    
    currentDirCluster = 2;
    syscall(3, currentDirCluster,(uint32_t) &currentDir, 1);
    
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
