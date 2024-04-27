#include <command.h>
#include <io.h>
#include "header/stdlib/string.h"

bool strcmp(char *a, char *b, int length) {
    for(int i = 0; i < length; i++) {
        if(a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

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
    if(strcmp(buf, clear, 4)) {
        clear_screen();
        set_cursor(0, 0);
    } else if (strcmp(buf, cd, 1)) {
        // cd
        puts("\n\ncd");
    } else if (strcmp(buf, ls, 1)) {
        // ls
        puts("\n\nls");
    } else if (strcmp(buf, mkdir, 4)) {
        // mkdir
        puts("\n\nmkdir");
    } else if (strcmp(buf, cat, 3)) {
        // cat
        puts("\n\ncat");
    } else if (strcmp(buf, cp, 1)) {
        // cp
        puts("\n\ncp");
    } else if (strcmp(buf, rm, 1)) {
        // rm
        puts("\n\nrm");
    } else if (strcmp(buf, mv, 1)) {
        // mv
        puts("\n\nmv");
    } else if (strcmp(buf, find, 4)) {
        // find
        puts("\n\nfind");
    } else if (strcmp(buf, help, 4)) {
        help_command();
    }
    else {
        puts("\nCommand not found\n");
    }
}