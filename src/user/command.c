#include <command.h>
#include <cwdlist.h>
#include <system.h>
#include <io.h>
#include <string.h>
#include "header/filesystem/fat32.h"

#define MAX_COMMAND_ARGS 20
#define MAX_ARGS_LENGTH 200

uint8_t extract_args(char* line, char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH]){
    uint8_t i = 0;
    // Skip first spaces
    while(line[i] == ' ') i++;

    uint8_t curr_char = 0;
    uint8_t curr_arg = 0;

    uint8_t last_curr_char = 0;
    while(line[i] != '\0') {
        if(line[i] != ' '){
            args[curr_arg][curr_char] = line[i];
            curr_char++;
            last_curr_char = curr_char;
            i++;
        } else {
            // Skip spaces
            while(line[i] == ' ') i++;
            curr_arg++;
            curr_char = 0;
        }
    }

    // Clean input
    uint8_t j = last_curr_char;
    while(j < MAX_ARGS_LENGTH) {
        args[curr_arg][j] = '\0';
        j++;
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
    char folderName[DIR_NAME_LENGTH];
    char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH];
    extract_args(cd, args);
    memcpy(folderName, args[1], DIR_NAME_LENGTH);
    if(folderName[0] == '\0'){
        puts("\nPlease provide folder name\n");
        return;
    }

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
        if(memcmp(dir_table.table[i].name, folderName, DIR_NAME_LENGTH) == 0){
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
    extract_args(buf, args);
    memcpy(folderName, args[1], DIR_NAME_LENGTH);
    if(folderName[0] == '\0'){
        puts("\nPlease provide folder name\n");
        return;
    }

    uint8_t error_code = make_directory(folderName, current_parent_cluster(cwd_list));
    if(error_code != 0) {
        puts("\nFolder ");
        puts(folderName);
        puts(" already exist");
    }
}

void handle_cat(char* buf, struct CWDList* cwd_list){
    char fileName[DIR_NAME_LENGTH];
    char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH];
    extract_args(buf, args);
    memcpy(fileName, args[1], DIR_NAME_LENGTH);
    if(fileName[0] == '\0'){
        puts("\nPlease provide file name\n");
        return;
    }

    struct FAT32DirectoryEntry entry;
    memcpy(entry.name, fileName, DIR_NAME_LENGTH); // kano
    memcpy(entry.ext, "\0\0\0", 3);
    uint32_t content_size = 2048;

    uint8_t error_code;
    while(true) {
        entry.filesize = content_size;
        char content[content_size];
        // Error code: 0 success - 1 not a file - 2 not enough buffer - 3 not found - -1 unknown
        error_code = read_file(&entry, current_parent_cluster(cwd_list), content);
        if(error_code == 1) {
            puts("\n");
            puts(fileName);
            puts(" is not a file");
            return;
        } else if(error_code == 2) {
            content_size += 2048;
            continue;
        } else if(error_code == 3) {
            puts("\n");
            puts(fileName);
            puts(" not found");
            continue;
        } else if(error_code == -1) {
            puts("\nUnknown error has occured");
            continue;
        }

        puts("\n");
        puts(content);
        break;
    }

}

void handle_rm(char* buf, struct CWDList* cwd_list){
    char fileName[DIR_NAME_LENGTH];
    char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH];
    extract_args(buf, args);
    memcpy(fileName, args[1], DIR_NAME_LENGTH);

    struct FAT32DirectoryEntry entry = {
        .filesize = 0xFFFF,
    };

    memcpy(entry.name, fileName, DIR_NAME_LENGTH);
    memcpy(entry.ext, "\0\0\0", 3);

    uint8_t error_code = delete_file_or_dir(&entry, current_parent_cluster(cwd_list));
    // Error code: 0 success - 1 not found - 2 folder is not empty - -1 unknown
    if(error_code == 0) {
        puts("\nDeleted ");
        puts(entry.name);
        puts("\n");
    } else if(error_code == 1){
        puts("\nFolder not found\n");
    } else if(error_code == 2){
        puts("\nFolder is not empty\n");
    } else if(error_code == -1){
        puts("\nUnknown error has occured\n");
    }
}

void handle_cp(char* buf, struct CWDList* cwd_list){
    char src[MAX_ARGS_LENGTH];
    char dest[MAX_ARGS_LENGTH];
    char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH];
    extract_args(buf, args);
    memcpy(src, args[1], MAX_ARGS_LENGTH);
    memcpy(dest, args[2], MAX_ARGS_LENGTH);

    if(src[0] == '\0'){
        puts("\nPlease provide source file name\n");
        return;
    }
    if(dest[0] == '\0'){
        puts("\nPlease provide destination file name\n");
        return;
    }

    struct FAT32DirectoryEntry entry;
    memcpy(entry.name, src, DIR_NAME_LENGTH);
    memcpy(entry.ext, "\0\0\0", 3);
    uint32_t content_size = 2048;


    // Read src
    uint8_t error_code;
    while(true) {
        entry.filesize = content_size;
        char content[content_size];
        error_code = read_file(&entry, current_parent_cluster(cwd_list), content);
        if(error_code == 1) {
            puts("\n");
            puts(src);
            puts(" is not a file");
            return;
        } else if(error_code == 2) {
            content_size += 2048;
            continue;
        } else if(error_code == 3) {
            puts("\n");
            puts(src);
            puts(" not found");
            continue;
        } else if(error_code == -1) {
            puts("\nUnknown error has occured");
            continue;
        }


        // Success
        // Write dest
        struct FAT32DirectoryEntry copy;
        copy.filesize = entry.filesize;
        memcpy(copy.name, dest, DIR_NAME_LENGTH); // TODO: from back because src is a path
        memcpy(copy.ext, "\0\0\0", 3); // TODO: extract extension from file name
        write_file(&copy, current_parent_cluster(cwd_list), content); // TODO: get the current_parent_cluster based on the path. Right now its the cwd. Just keep reading the directory to find it

        break;
    }


}

void handle_mv(char* buf, struct CWDList* cwd_list){
    char src[MAX_ARGS_LENGTH];
    char dest[MAX_ARGS_LENGTH];
    char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH];
    extract_args(buf, args);
    memcpy(src, args[1], MAX_ARGS_LENGTH);
    memcpy(dest, args[2], MAX_ARGS_LENGTH);
}


// each folder wont contain more than 1 file/folder with same name.
void recursive_find(struct FAT32DirectoryTable* dir_table, char file_name[MAX_ARGS_LENGTH], uint32_t cluster_number, struct CWDList* cwd_list){ // depth is just for debugging

    for(uint32_t i = 2; i < 64; i++){
        if(is_empty(&dir_table->table[i])) continue;
        
        
        if(memcmp(dir_table->table[i].name, file_name, DIR_NAME_LENGTH) == 0 /* && memcmp(dir_table.table[i].name, file_name, DIR_EXT_LENGTH) == 0 */ ){
            // traversal back to populate cwd_list 
            
        }

        if(is_directory(&dir_table->table[i])) {
            struct FAT32DirectoryTable children;
            get_dir(dir_table->table[i].name, cluster_number, &children);
            recursive_find(&children, file_name, dir_table->table[i].cluster_low, cwd_list);
        }


        // print from root for debugging
        // else {
        //     uint16_t parent_cluster = cluster_number;
        //     struct FAT32DirectoryTable children;
        //     struct CWDList list;
        //     // puts(dir_table->table[i].name);
        //     // puts("/");
        //     // puts(dir_table->table[0].name);
        //     // puts("/");
        //     push_dir(&list, dir_table->table[i].name, 0);
        //     push_dir(&list, dir_table->table[0].name, 0);
            
        //     while(parent_cluster != ROOT_CLUSTER_NUMBER){
        //         get_dir("..\0\0\0\0\0\0", parent_cluster, &children);
        //         parent_cluster = children.table[1].cluster_low;
        //         // puts("/");
        //         // puts(children.table[0].name);
        //         push_dir(&list, children.table[0].name, 0);
        //     }
        //     print_path_to_cwd(&list);
        //     puts("\n");
        // }
    }
}

void handle_find(char* buf){
    char file_name[MAX_ARGS_LENGTH]; // TODO: validate if this is not a path
    char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH];
    extract_args(buf, args);
    memcpy(file_name, args[1], MAX_ARGS_LENGTH);
    
    struct CWDList cwd_list[100];
    struct FAT32DirectoryTable dir_table;
    get_dir("root\0\0\0\0", ROOT_CLUSTER_NUMBER, &dir_table);
    puts("\n");
    recursive_find(&dir_table, file_name, ROOT_CLUSTER_NUMBER, &cwd_list);

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
        handle_cat(buf, cwd_list);
    } else if (memcmp(buf, cp, 2) == 0) {
        handle_cp(buf, cwd_list);
    } else if (memcmp(buf, rm, 2) == 0) {
        handle_rm(buf, cwd_list);
    } else if (memcmp(buf, mv, 2) == 0) {
        handle_mv(buf, cwd_list);
    } else if (memcmp(buf, find, 4) == 0) {
        handle_find(buf);
    } else if (memcmp(buf, help, 4) == 0) {
        help_command();
    } else {
        puts("\nCommand ");
        puts(buf);
        puts(" not found\n");
    }
}