#include <command.h>
#include <system.h>
#include <io.h>
#include <string.h>
#include "header/filesystem/fat32.h"



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

void handle_cd(char *cd) {
    char folderName[8];
    memcpy(folderName, cd + 3, 8);

    if(memcmp(folderName, "..", 2) == 0) {
        if(is_in_root()) {
            puts("\n");
            puts("Already in root directory\n");
            return;
        }
        change_directory("root\0\0\0\0");
        return;
    }

    struct FAT32DirectoryTable dir_table;
    get_dir(&dir_table, folderName);
    if(is_directory(&dir_table.table[0])) {
        change_directory(folderName);
        puts("\n");
    } else {
        puts("\n");
        puts(folderName);
        puts(" is not a directory\n");
    }
}

void handle_ls() {
    char dirs[MAX_DIR_LENGTH][DIR_NAME_LENGTH];
    uint32_t len = DIR_NAME_LENGTH;
    struct FAT32DirectoryTable dir_table;
    get_cwd(&dir_table);
    puts("\n");
    for(uint32_t i = 2; i < MAX_DIR_LENGTH; i++){
        if(is_empty(&dir_table.table[i])) break;
        puts(dir_table.table[i].name);
        puts("\n");
    }
}

void handle_mkdir(char *buf) {
    char folderName[DIR_NAME_LENGTH];
    memcpy(folderName, buf + 6, 8);
    make_directory(folderName);
    puts("\n");
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
        handle_cd(buf);
    } else if (memcmp(buf, ls, 2) == 0) {
        handle_ls();
    } else if (memcmp(buf, mkdir, 4) == 0) {
        handle_mkdir(buf);

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