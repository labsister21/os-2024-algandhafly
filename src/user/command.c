#include <command.h>
#include <directorystack.h>
#include <system.h>
#include <io.h>
#include <string.h>
#include "header/filesystem/fat32.h"

#define MAX_COMMAND_ARGS 20
#define MAX_ARGS_LENGTH 200
#define not !
#define but &&
#define and &&
#define or ||
#define DOT '.'
#define FWSLASH '/'
#define NULL_CHAR '\0'
#define state uint8_t;
#define _GLOBAL_OFFSET_TABLE_ 727;

bool is_alpha_numeric(char c) {
    bool is_number = ( c >= 48 and c <= 57 );
    bool is_lowercase = ( c >= 65 and c <= 90);
    bool is_uppercase = ( c >= 97 and c <= 122);
    bool is_underscore = ( c == 95 );

    return is_number or is_lowercase or is_uppercase or is_underscore;
}

uint8_t path_to_dir_stack(char* path, struct DirectoryStack* dir_stack){

    int i;
    char* c = path;
    int infinite_loop_guard = 0;

    if (c[0] == DOT and c[1] == FWSLASH) {
        c++;++c;
    }

    char name[DIR_NAME_LENGTH];
    char ext[DIR_EXT_LENGTH];
    int l = 0;
    int e = 0;
    bool name_state = 0, extension_state = 1;
    bool current_state = name_state;

    while (1) {

        char ch = *(c++);

        if (ch == FWSLASH or ch == NULL_CHAR) {
            
            if ((not e) but current_state == extension_state) {
                goto EXTENSION_EMPTY;
            } 

            if (l > 0) {
                struct FAT32DirectoryEntry entry;
                for (i = 0; i < DIR_NAME_LENGTH; i++) {
                    entry.name[i] = (i < l ? name[i] : NULL_CHAR);
                }
                for (i = 0; i < DIR_EXT_LENGTH; i++) {
                    entry.ext[i] = (i < e ? ext[i] : NULL_CHAR);
                }
                push_dir(dir_stack, &entry);
            }
            else if (l == 0 and e != 0) { // found extension but no filename
                goto FILENAME_EMPTY;
            }
            
            l = 0;
            e = 0;
            current_state = name_state;
        }

        else if (ch == DOT) {

            if (current_state == extension_state and l == 0) { // handle double dots ; i.e. "somethingsomething/.."

                // if next char doesnt end this current dir, then it dies, because double dots must be JUST double dots, no addons like "/..a/" or even ".../"
                if (*c != FWSLASH and *c != NULL_CHAR) {
                    goto INVALID_NAME;
                }
                
                else ;
                struct FAT32DirectoryEntry entry = {
                    .name = "..\0\0\0\0\0\0",
                    .ext = "\0\0\0",
                };
                push_dir(dir_stack, &entry);
                current_state = name_state;
                c++;
                l = 0; e = 0;
            }
            else {
                current_state = extension_state;
            }

        }

        else {
            if (not is_alpha_numeric(ch)) {
                if (current_state == name_state) goto INVALID_NAME;
                else goto INVALID_EXTENSION;
            }
            else if (current_state == name_state) {
                if (l < DIR_NAME_LENGTH) name[l++] = ch;
                else goto TOO_LONG_FILENAME;
            }
            else if (current_state == extension_state) {
                if (e < DIR_EXT_LENGTH) ext[e++] = ch;
                else goto TOO_LONG_EXTENSION;
            }
        }
        
        if (ch == NULL_CHAR) break;
        if (++infinite_loop_guard > 10000) goto INFINITE_LOOP;
    }

    return 0;
    
    INVALID_NAME:
    return 1;
    
    INVALID_EXTENSION:
    return 2;
    
    TOO_LONG_FILENAME:
    return 3;
    
    EXTENSION_EMPTY:
    return 4;
    
    FILENAME_EMPTY:
    return 5;

    TOO_LONG_EXTENSION: 
    return 6;

    INFINITE_LOOP: 
    return 7;

    /**
     * TODO: validate path?
    */

}

// dir_stack_cwd: the cwd passed from the user-shell
// dir_stack: the dir_stack to be populated
uint8_t path_to_dir_stack_from_cwd(char* path, struct DirectoryStack* dir_stack_cwd, struct DirectoryStack* dir_stack) {
    uint8_t error_code = path_to_dir_stack(path, dir_stack);
    if(error_code != 0) return error_code;

    uint16_t cwd_cluster = current_parent_cluster(dir_stack_cwd);
    struct FAT32DirectoryTable dir_table;

    
    for(uint16_t curr_idx = 0; curr_idx < dir_stack->length; curr_idx++) {
        if(dir_stack->entry[curr_idx].ext[0] == NULL_CHAR)
            get_dir(dir_stack->entry[curr_idx].name, cwd_cluster, &dir_table);
        else
            get_file_dir(dir_stack->entry[curr_idx].name, dir_stack->entry[curr_idx].ext, cwd_cluster, &dir_table);

        bool found = false;
        for(uint8_t i = 0; i < 64; i++) {
            if(is_empty(&dir_table.table[i])) continue;
            if(memcmp(dir_table.table[i].name, dir_stack->entry[curr_idx].name, DIR_NAME_LENGTH) == 0) {
                memcpy(&dir_stack->entry[curr_idx], &dir_table.table[i], sizeof(struct FAT32DirectoryEntry));
                cwd_cluster = dir_table.table[i].cluster_low;
                found = true;
                break;
            }
        }
        if(!found) return 8; // Case one of them not found
    }
    return 0;
}

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

void handle_cd(char *cd, struct DirectoryStack* dir_stack) {
    char folderName[DIR_NAME_LENGTH];
    char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH];
    extract_args(cd, args);
    if(args[1][0] == '\0'){
        puts("\nPlease provide folder name\n");
        return;
    }

    struct DirectoryStack* input_path;
    path_to_dir_stack(args[1], input_path);


    // uint8_t error_code = path_to_dir_stack_from_cwd(args[1], dir_stack, input_path);
    // if(error_code != 0) {
    //     puts("\nInvalid path\n");
    // }

    // for(uint8_t i = 0; i < input_path->length; i++) {
    //     push_dir(dir_stack, &input_path->entry[i]);
    // }
    

    // if(memcmp(folderName, "..", 2) == 0) {
    //     if(memcmp(last_dir(dir_stack), "root", 4) == 0) {
    //         puts("\n");
    //         puts("Already in root directory\n");
    //         return;
    //     }
    //     pop_dir(dir_stack);
    //     return;
    // }
    
    // struct FAT32DirectoryTable dir_table;
    // get_dir(last_dir(dir_stack), prev_parent_cluster(dir_stack), &dir_table);
    // for(uint32_t i = 2; i < 64; i++){
    //     if(is_empty(&dir_table.table[i])) continue;
    //     if(memcmp(dir_table.table[i].name, folderName, DIR_NAME_LENGTH) == 0){
    //         if(is_directory(&dir_table.table[i])) {
    //             push_dir(dir_stack, &dir_table.table[i]);
    //             return;
    //         } else {
    //             puts("\n");
    //             puts(folderName);
    //             puts(" is not a folder.");
    //             return;
    //         }
    //         break;
    //     }
    // }
    // puts("\n");
    // puts("Folder ");
    // puts(folderName);
    // puts(" not found");
}

void handle_ls(struct DirectoryStack* dir_stack) {
    struct FAT32DirectoryTable dir_table;
    get_dir(last_dir(dir_stack), prev_parent_cluster(dir_stack), &dir_table);

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

void handle_mkdir(char *buf, struct DirectoryStack* dir_stack) {
    char folderName[DIR_NAME_LENGTH];
    char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH];
    extract_args(buf, args);
    memcpy(folderName, args[1], DIR_NAME_LENGTH);
    if(folderName[0] == '\0'){
        puts("\nPlease provide folder name\n");
        return;
    }

    uint8_t error_code = make_directory(folderName, current_parent_cluster(dir_stack));
    // Error code: 0 success - 1 file/folder already exist - 2 invalid parent cluster - -1 unknown
    if(error_code == 0) {
        puts("\nCreated folder ");
        puts(folderName);
        puts("\n");
    } else if(error_code == 1) {
        puts("\nFolder ");
        puts(folderName);
        puts(" already exist");
    } else if(error_code == 2) {
        puts_color("\nError: invalid parent cluster", Color_Red, Color_Black);
    } else {
        puts_color("Error: unknown", Color_Red, Color_Black);
    }
}

void handle_cat(char* buf, struct DirectoryStack* dir_stack){
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
        error_code = read_file(&entry, current_parent_cluster(dir_stack), content);
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
            return;
        } else if(error_code == -1) {
            puts("\nUnknown error has occured");
            return;
        }

        puts("\n");
        puts(content);
        break;
    }

}

void handle_rm(char* buf, struct DirectoryStack* dir_stack){
    char fileName[DIR_NAME_LENGTH];
    char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH];
    extract_args(buf, args);
    memcpy(fileName, args[1], DIR_NAME_LENGTH);

    struct FAT32DirectoryEntry entry = {
        .filesize = 0xFFFF,
    };

    memcpy(entry.name, fileName, DIR_NAME_LENGTH);
    memcpy(entry.ext, "\0\0\0", 3);

    uint8_t error_code = delete_file_or_dir(&entry, current_parent_cluster(dir_stack));
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

void handle_cp(char* buf, struct DirectoryStack* dir_stack){
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
        error_code = read_file(&entry, current_parent_cluster(dir_stack), content);
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
            return;
        } else if(error_code == -1) {
            puts("\nUnknown error has occured");
            return;
        }


        // Success
        // Write dest
        struct FAT32DirectoryEntry copy;
        copy.filesize = entry.filesize;
        memcpy(copy.name, dest, DIR_NAME_LENGTH); // TODO: from back because src is a path
        memcpy(copy.ext, "\0\0\0", 3); // TODO: extract extension from file name
        write_file(&copy, current_parent_cluster(dir_stack), content); // TODO: get the current_parent_cluster based on the path. Right now its the cwd. Just keep reading the directory to find it

        break;
    }


}

void handle_mv(char* buf, struct DirectoryStack* dir_stack){
    char src[MAX_ARGS_LENGTH];
    char dest[MAX_ARGS_LENGTH];
    char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH];
    extract_args(buf, args);
    memcpy(src, args[1], MAX_ARGS_LENGTH);
    memcpy(dest, args[2], MAX_ARGS_LENGTH);
}


// each folder wont contain more than 1 file/folder with same name.
void recursive_find(struct FAT32DirectoryTable* dir_table, char file_name[MAX_ARGS_LENGTH], uint32_t cluster_number, struct DirectoryStack* dir_stack){ // depth is just for debugging

    for(uint32_t i = 2; i < 64; i++){
        if(is_empty(&dir_table->table[i])) continue;
        
        
        if(memcmp(dir_table->table[i].name, file_name, DIR_NAME_LENGTH) == 0 /* && memcmp(dir_table.table[i].name, file_name, DIR_EXT_LENGTH) == 0 */ ){
            // traversal back to populate dir_stack 
            
        }

        if(is_directory(&dir_table->table[i])) {
            struct FAT32DirectoryTable children;
            get_dir(dir_table->table[i].name, cluster_number, &children);
            recursive_find(&children, file_name, dir_table->table[i].cluster_low, dir_stack);
        }


        // print from root for debugging
        else {
            uint16_t parent_cluster = cluster_number;
            struct FAT32DirectoryTable children;
            struct DirectoryStack list = {.length = 0};
            push_dir(&list, &dir_table->table[i]);
            push_dir(&list, &dir_table->table[0]);
            
            while(parent_cluster != ROOT_CLUSTER_NUMBER){
                get_dir("..\0\0\0\0\0\0", parent_cluster, &children);
                parent_cluster = children.table[1].cluster_low;
                push_dir(&list, &children.table[0]);
            }
            print_path_to_cwd(&list);
            puts("\n");
        }
    }
}

void handle_find(char* buf){
    char file_name[MAX_ARGS_LENGTH]; // TODO: validate if this is not a path
    char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH];
    extract_args(buf, args);
    memcpy(file_name, args[1], MAX_ARGS_LENGTH);
    
    struct DirectoryStack dir_stack[100];
    struct FAT32DirectoryTable dir_table;
    get_dir("root\0\0\0\0", ROOT_CLUSTER_NUMBER, &dir_table);
    puts("\n");
    recursive_find(&dir_table, file_name, ROOT_CLUSTER_NUMBER, &dir_stack);

}

const char clear[6] = "clear";
const char help[5] = "help";
const char cd[3] = "cd"; // cd	- Mengganti current working directory (termasuk .. untuk naik)
const char ls[3] = "ls\0"; // ls	- Menuliskan isi current working directory
const char mkdir[6] = "mkdir"; // mkdir	- Membuat sebuah folder kosong baru
const char cat[4] = "cat"; // cat	- Menuliskan sebuah file sebagai text file ke layar (Gunakan format LF newline)
const char cp[3] = "cp"; // cp	- Mengcopy suatu file (Folder menjadi bonus)
const char rm[3] = "rm"; // rm	- Menghapus suatu file (Folder menjadi bonus)
const char mv[3] = "mv"; // mv	- Memindah dan merename lokasi file/folder
const char find[5] = "find"; // find	- Mencari file/folder dengan nama yang sama diseluruh file system

void command(char *buf, struct DirectoryStack* dir_stack) {
    while(*buf == ' ') buf ++; // Skip spaces

    if(memcmp(buf, clear, 5) == 0) {
        clear_screen();
        set_cursor(0, 0);
        return; // prevent new line
    } else if (memcmp(buf, cd, 2) == 0) {
        handle_cd(buf, dir_stack);
    } else if (memcmp(buf, ls, 2) == 0) {
        handle_ls(dir_stack);
    } else if (memcmp(buf, mkdir, 5) == 0) {
        handle_mkdir(buf, dir_stack);
    } else if (memcmp(buf, cat, 3) == 0) {
        handle_cat(buf, dir_stack);
    } else if (memcmp(buf, cp, 2) == 0) {
        handle_cp(buf, dir_stack);
    } else if (memcmp(buf, rm, 2) == 0) {
        handle_rm(buf, dir_stack);
    } else if (memcmp(buf, mv, 2) == 0) {
        handle_mv(buf, dir_stack);
    } else if (memcmp(buf, find, 4) == 0) {
        handle_find(buf);
    } else if (memcmp(buf, help, 4) == 0) {
        help_command();
    } else if(buf[0] == '\0'){
        puts("\n");
        return; // prevent new line
    } else {
        puts("\nCommand ");
        puts(buf);
        puts(" not found\n");
    }
    puts("\n");
    
}