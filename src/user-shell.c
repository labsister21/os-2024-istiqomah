#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

uint32_t currentDirCluster;
struct FAT32DirectoryTable currentDir;
struct StringN currentDirPath;

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

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

void cetak_prompt() {
    char* os1 = "Istiqomah@OS-IF2230";
    char* os4 = ":";
    char* os2 = "/";
    char* os3 = "$";
    puts(os1, 0x5);
    puts(os4, 0x7);
    puts(os2, 0x9);
    puts(os3, 0x7);
}

void mkdir(struct StringN folder_Name){
    struct FAT32DriverRequest request = {
        .name = "\0\0\0\0\0\0\0\0",
        .parent_cluster_number = currentDirCluster,
        .buffer_size = 0,
    };
    if(folder_Name.len > 8){
        puts("Directory name is too long! (Max Characters is 8)", 0xC);
    }
    else{
        for (uint8_t i = 0; i < folder_Name.len; i++) {
            request.name[i] = folder_Name.buf[i];
        }
        int8_t retcode;
        syscall(2, (uint32_t) &request, (uint32_t) &retcode, 0);
        switch (retcode) {
        case 0:
            puts("Make directory success! " , 0x2);
            puts("'", 0x2);
            puts(folder_Name.buf, 0x2);
            puts("'", 0x2);
            puts(" has been created..\n", 0x2);

            syscall(3, currentDirCluster, (uint32_t) &currentDir, 1);

            break;
        case 1:
            puts("mkdir: cannot create directory ", 0xC);
            puts("'", 0xC);

            puts(folder_Name.buf, 0xC);
            puts("'", 0xC);
            
            puts(": File exists\n", 0xC);
            
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
        puts("rm: cannot remove : name is too long! (Max Characters is 8)", 0xC);
    }
    else{
       for (uint8_t i = 0; i < folder.len; i++) {
            request.name[i] = folder.buf[i];
        }
        int8_t retcode;
        syscall(8,(uint32_t) &request, (uint32_t) &retcode, 0);
        switch (retcode){
            case 0:
                puts("Remove success! ", 0x2);
                puts("'", 0x2);
                puts(folder.buf, 0x2);
                puts("'", 0x2);
                puts(" has been removed..\n", 0x2);
                break;
            case 1:
                puts("rm: cannot remove '", 0xC);
                puts(folder.buf, 0xC);
                puts("': No such file or directory", 0xC);
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
        puts(entry.name, 0x2);
        puts(" ", 0x2);
    }
    puts("\n", 0x2);
}

void cp (struct StringN filename) {
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

void cd(struct StringN dirname) {
    if (dirname.len > 8) {
        puts("Directory name is too long! (Maximum 8 Characters)", 0xC);
        return;
    }

    for (unsigned int i = 0; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++) {
        struct FAT32DirectoryEntry entry = currentDir.table[i];
        if (entry.name[0] != '\0') {
            char fullname[9];
            memcpy(fullname, entry.name, 8);
            fullname[8] = '\0'; 

            if (strcmp(dirname.buf, "..") == 1) {
                // mengembalikan ke parent directory
                if (currentDirCluster == 2) {
                    puts("Already in root directory\n", 0xC);
                    return;
                } else {
                    currentDirCluster = currentDir.parent_cluster_number[currentDir.buffer_index - 1];
                    currentDir.buffer_index--;

                    // Load the parent directory's table into currentDir
                    syscall(1, currentDirCluster, (uint32_t) &currentDir, 1);
                    puts("Changed directory to parent directory\n", 0x2);
                    return;
                }
            } else if (!strcmp(fullname, dirname.buf) == 0 && (entry.attribute & 0x10)) {
                // mencari tempat di currentDir.parent_cluster_number yang kosong dari paling kiri, lalu memasukkan currentDirCluster

                currentDir.parent_cluster_number[currentDir.buffer_index] = currentDirCluster;
                currentDir.buffer_index++;

                // Update currentDirCluster to the new directory's cluster number
                currentDirCluster = entry.cluster_low | (entry.cluster_high << 16);


                // Load the new directory's table into currentDir
                syscall(1, currentDirCluster, (uint32_t) &currentDir, 1);
                puts("Changed directory to ", 0x2);
                puts(entry.name, 0x2);
                puts("\n", 0x2);
                return;
            }
        }
    }
    puts("Directory not found\n", 0xC);
}

void find(struct StringN filename) {
    for (unsigned int i = 0; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++) {
        struct FAT32DirectoryEntry entry = currentDir.table[i];
        if (entry.name[0] != '\0') {
            char fullname[9];
            memcpy(fullname, entry.name, 8);
            fullname[8] = '\0'; 

            // puts filename with the format "File found in cluster <cluster_number>: <filename>"
            if (strcmp(fullname, filename.buf) == 1) {
                puts("File found: ", 0x2);
                puts(fullname, 0x2);
                puts("\n", 0x2);
                return;
            }
        }
    }
    puts("File not found\n", 0xC);
}

void mv(struct StringN filename, struct StringN dest) {
    // memindahkan file dari currentDirCluster ke dest
    // jika dest adalah direktori, maka file dipindahkan ke dalam direktori tersebut
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

    int retcode;
    syscall(0, (uint32_t) &request, (uint32_t) &retcode, 0);
    struct FAT32DriverRequest copy = {
        .buf = (uint8_t*) request.buf,
        .name = "copy",
        .ext = "\0\0\0",
        .parent_cluster_number = currentDirCluster,
        .buffer_size = strlen(copy.buf),
    };

    if (dest.len > 8) {
        puts("Destination name is too long! (Maximum 8 Characters)", 0xC);
        return;
    }

    for (unsigned int i = 0; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++) {
        struct FAT32DirectoryEntry entry = currentDir.table[i];
        if (entry.name[0] != '\0') {
            char fullname[9];
            memcpy(fullname, entry.name, 8);
            fullname[8] = '\0'; 

            if (!strcmp(fullname, filename.buf) == 0) {
                // File found, now check if dest is a directory
                if (entry.attribute == 0x0) {
                    // Move file to the destination directory
                    cd(dest);
                    int retcode;
                    syscall(2, (uint32_t) &copy.buf, (uint32_t) &retcode, 0);
                    switch (retcode) {
                        case 0:
                            puts("File moved successfully\n", 0x2);
                            // syscall(8, (uint32_t) &request, (uint32_t) &retcode, 0);
                            break;
                        case 1:
                            puts("mv: cannot move file '", 0xC);
                            puts(filename.buf, 0xC);
                            puts("': No such file or directory\n", 0xC);
                            break;
                        default:
                            break;
                    }
                    return;
                } else {
                    puts("mv: destination is not a directory\n", 0xC);
                    return;
                }
            }
        }
    }
    puts("File not found\n", 0xC);
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
        if (input.buf[i] == ' ') {
            break;
        }
        stringn_appendchar(&variabel, input.buf[i]);
    }

    struct StringN variabel2;
    stringn_create(&variabel2);
    i++;
    for (i = i; i < input.len; i++) {
        if (input.buf[i] == ' ') {
            break;
        }
        stringn_appendchar(&variabel2, input.buf[i]);
    }
    

    if (memcmp(perintah.buf, "cd", 2) == 0)
    {
        puts("\n", 0x7);
        cd(variabel);
        cetak_prompt();
    } 
    else if (memcmp(perintah.buf, "ls", 2) == 0)
    {
        puts("\n", 0x7);
        ls();
        cetak_prompt();
    }
    else if (memcmp(perintah.buf, "mkdir", 5) == 0)
    {
        puts("\n", 0x7);
        mkdir(variabel);
        cetak_prompt();
    }
    else if (memcmp(perintah.buf, "cat", 3) == 0)
    {
        puts("\n", 0x7);
        cat(variabel);
        cetak_prompt();
    }
    else if (memcmp(perintah.buf, "rm", 2) == 0)
    {
        puts("\n", 0x7);
        rm(variabel);
        cetak_prompt();
    }
    else if (memcmp(perintah.buf, "cp", 2) == 0)
    {
        puts("\n", 0x7);
        cp(variabel);
        cetak_prompt();
    }
    else if (memcmp(perintah.buf, "find", 4) == 0)
    {
        puts("\n", 0x7);
        find(variabel);
        cetak_prompt();
    }
    else if (memcmp(perintah.buf, "mv", 2) == 0)
    {
        puts("\n", 0x7);
        mv(variabel, variabel2);
        cetak_prompt();
    }
    else
    {
        puts("\nCommand not found\n", 0xC);
        cetak_prompt();
    }
}

int main(void) {    
    currentDirCluster = 2;
    currentDir.buffer_index = 0;
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
