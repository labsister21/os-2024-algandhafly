#include <command.h>
#include <system.h>
#include <io.h>
#include <string.h>
#include "header/filesystem/fat32.h"


const char current_directory[DIR_NAME_LENGTH] = "root";
const char current_directory_path[MAX_DIR_LENGTH] = "/";

void help_command() {
    puts("\n\n");
    puts("clear : Clear the screen\n");
    puts("cd    : Change the current working directory\n");
    puts("ls    : List the contents of the current working directory\n");
    puts("mkdir : Create a new empty folder\n");
    puts("cat   : Write a file as a text file to the screen (Use LF newline format)\n");
    puts("cp    : Copy a file (Folder is a bonus)\n");
    puts("rm    : Delete a file (Folder is a bonus)\n");
    puts("mv    : Move and rename the location of a file/folder\n");
    puts("find  : Search for files/folders with the same name throughout the file system\n");
    puts("help  : Show this help\n\n");
}

void get_current_directory(struct FAT32DirectoryTable *dir_table) {
    struct FAT32DriverRequest request;
    request.parent_cluster_number = 2;
    memcpy(request.name, "root\0\0\0\0", 8);
    request.buf = dir_table;

    uint8_t error_code;
    systemCall(1, (uint32_t )&request, (uint32_t )&error_code, 0);
    if(error_code != 0) {
        puts("\nError code: "); put_int(error_code); puts("\n");
    }
}

bool is_empty(struct FAT32DirectoryEntry *entry) {
    return entry->user_attribute != UATTR_NOT_EMPTY;
}
void print_current_directory() {
    char dirs[MAX_DIR_LENGTH][DIR_NAME_LENGTH];
    struct FAT32DirectoryTable dir_table;
    get_current_directory(&dir_table);
    uint32_t len = DIR_NAME_LENGTH;
    puts("\n");
    for(uint32_t i = 0; i < MAX_DIR_LENGTH; i++){
        if(is_empty(&dir_table.table[i])) break;
        puts(dir_table.table[i].name);
        puts("\n");
    }
}

const char clear[5] = "clear";
const char help[5] = "help";
const char cd[2] = "cd"; // cd	- Mengganti current working directory (termasuk .. untuk naik)
const char ls[2] = "ls"; // ls	- Menuliskan isi current working directory
const char mkdir[5] = "mkdir"; // mkdir	- Membuat sebuah folder kosong baru
const char cat[3] = "cat"; // cat	- Menuliskan sebuah file sebagai text file ke layar (Gunakan format LF newline)
const char cp[2] = "cp"; // cp	- Mengcopy suatu file (Folder menjadi bonus)
const char rm[2] = "rm"; // rm	- Menghapus suatu file (Folder menjadi bonus)
const char mv[2] = "mv"; // mv	- Memindah dan merename lokasi file/folder
const char find[4] = "find"; // find	- Mencari file/folder dengan nama yang sama diseluruh file system

void command(char *buf) {
    if(memcmp(buf, clear, 4) == 0) {
        clear_screen();
        set_cursor(0, 0);
    } else if (memcmp(buf, cd, 2) == 0) {
        // cd
        puts("\n\ncd");
    } else if (memcmp(buf, ls, 2) == 0) {
        print_current_directory();
    } else if (memcmp(buf, mkdir, 4) == 0) {
        // mkdir
        puts("\n\nmkdir");
    } else if (memcmp(buf, cat, 3) == 0) {
        // cat
        puts("\n\ncat");
    } else if (memcmp(buf, cp, 2) == 0) {
        // cp
        puts("\n\ncp");
    } else if (memcmp(buf, rm, 2) == 0) {
        // rm
        puts("\n\nrm");
    } else if (memcmp(buf, mv, 2) == 0) {
        // mv
        puts("\n\nmv");
    } else if (memcmp(buf, find, 4) == 0) {
        // find
        puts("\n\nfind");
    } else if (memcmp(buf, help, 4) == 0) {
        help_command();
    }
    else {
        puts("\nCommand ");
        puts(buf);
        puts(" not found\n");
    }
}