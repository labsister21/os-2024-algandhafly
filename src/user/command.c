#include <command.h>
#include <cwdlist.h>
#include <system.h>
#include <io.h>
#include <string.h>
#include "header/filesystem/fat32.h"

#define MAX_COMMAND_ARGS 20
#define MAX_ARGS_LENGTH 100

uint8_t extract_args(char* line, char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH]){
    uint8_t i = 0;
    // Skip first spaces
    while(line[i] == ' ') i++;

    uint8_t curr_char = 0;
    uint8_t curr_arg = 0;
    while(line[i] != '\0') {
        if(line[i] != ' '){
            args[curr_arg][curr_char] = line[i];
            curr_char++;
            i++;
        } else {
            // Skip spaces
            while(line[i] == ' ') i++;
            curr_arg++;
            curr_char = 0;
        }
    }
    return curr_arg;
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

void handle_cd(char *cd, struct CWDList* cwd_list) {
    char folderName[8];
    char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH];
    extract_args(cd, args);
    memcpy(folderName, args[1], 8);

    if(memcmp(folderName, "..", 2) == 0) {
        if(memcmp(last_dir(cwd_list), "root", 4) == 0) {
            puts("\n");
            puts("Already in root directory\n");
            return;
        }
        pop_dir(cwd_list);
        return;
    }
    
    struct FAT32DirectoryTable dir_table;
    get_dir(last_dir(cwd_list), prev_parent_cluster(cwd_list), &dir_table);
    for(uint32_t i = 2; i < 64; i++){
        if(is_empty(&dir_table.table[i])) continue;
        if(memcmp(dir_table.table[i].name, folderName, 8) == 0){
            if(is_directory(&dir_table.table[i])) {
                push_dir(cwd_list, folderName, dir_table.table[i].cluster_low);
                return;
            } else {
                puts("\n");
                puts(folderName);
                puts(" is not a folder.");
                return;
            }
            break;
        }
    }
    puts("\n");
    puts("Folder ");
    puts(folderName);
    puts(" not found");
}

void handle_ls(struct CWDList* cwd_list) {
    struct FAT32DirectoryTable dir_table;
    get_dir(last_dir(cwd_list), prev_parent_cluster(cwd_list), &dir_table);

    bool has_any = false;
    puts("\n");
    for(uint32_t i = 2; i < 64; i++){
        if(is_empty(&dir_table.table[i])) continue;
        has_any = true;

        if(is_directory(&dir_table.table[i])) {
            puts_color(dir_table.table[i].name, Color_LightCyan, Color_Black);
        } else {
            puts_color(dir_table.table[i].name, Color_LightBlue, Color_Black);
        }
        puts("\n");
    }
    if(!has_any){
        puts("Empty folder");
    }
}

void handle_mkdir(char *buf, struct CWDList* cwd_list) {
    char folderName[DIR_NAME_LENGTH];
    char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH];
    extract_args(folderName, args);
    memcpy(folderName, args[1], 8);

    uint8_t error_code = make_directory(folderName, current_parent_cluster(cwd_list));
    if(error_code != 0) {
        puts("\nError Code: ");
        put_int(error_code);
    }
}

void handle_cat(char* buf){
    char fileName[DIR_NAME_LENGTH];
    char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH];
    extract_args(buf, args);
    memcpy(fileName, args[1], 8);

    struct FAT32DirectoryEntry entry;
    memcpy(entry.name, fileName, 8); // kano
    memcpy(entry.ext, "\0\0\0", 3);
    uint32_t content_size = 2048;

    uint8_t error_code;
    while(true) {
        entry.filesize = content_size;
        char content[content_size];
        error_code = read_file(&entry, content);
        if(error_code == 2) {
            content_size += 2048;
            continue;
        }

        puts("\n");
        puts(content);
        break;
    }

}

void handle_rm(char* buf){
    char fileName[DIR_NAME_LENGTH];
    memcpy(fileName, (char*)(buf + 3), 8);

    struct FAT32DirectoryEntry entry = {
        .filesize = 0xFFFF,
    };

    memcpy(entry.name, fileName, 8);
    memcpy(entry.ext, "\0\0\0", 3);

    uint8_t error_code = delete_file_or_dir(&entry);
    if(error_code != 0) {
        puts("\nError Code: ");
        put_int(error_code);
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

void command(char *buf, struct CWDList* cwd_list) {
    while(*buf == ' ') buf++; // Skip spaces

    if(memcmp(buf, clear, 4) == 0) {
        clear_screen();
        set_cursor(0, 0);
    } else if (memcmp(buf, cd, 2) == 0) {
        handle_cd(buf, cwd_list);
    } else if (memcmp(buf, ls, 2) == 0) {
        handle_ls(cwd_list);
    } else if (memcmp(buf, mkdir, 4) == 0) {
        handle_mkdir(buf, cwd_list);
    } else if (memcmp(buf, cat, 3) == 0) {
        handle_cat(buf);
    } else if (memcmp(buf, cp, 2) == 0) {
        // cp
        puts("\n\ncp");
    } else if (memcmp(buf, rm, 2) == 0) {
        handle_rm(buf);
    } else if (memcmp(buf, mv, 2) == 0) {
        // mv
        puts("\n\nmv");
    } else if (memcmp(buf, find, 4) == 0) {
        // find
        puts("\n\nfind");
    } else if (memcmp(buf, help, 4) == 0) {
        help_command();
    } else {
        puts("\nCommand ");
        puts(buf);
        puts(" not found\n");
    }
}