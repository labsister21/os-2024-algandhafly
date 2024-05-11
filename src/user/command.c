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

    // Read current directory, dir_table contains all folder in current directory
    char* curr_dir = peek_top(dir_stack_cwd)->name;
    char* curr_ext = peek_top(dir_stack_cwd)->ext;
    get_dir(curr_dir, cwd_cluster, &dir_table);

    for(uint8_t k = 0; k < dir_stack->length; k++) {
        bool found = false;
        for(uint32_t i = 2; i < 64; i++){
            if(is_empty(&dir_table.table[i])) continue;
            if(memcmp(&dir_table.table[i].name, curr_dir, DIR_NAME_LENGTH) == 0 && memcmp(&dir_table.table[i].ext, curr_ext, DIR_EXT_LENGTH) == 0){
                cwd_cluster = dir_table.table[i].cluster_low;
                memcpy(&dir_stack->entry[k], &dir_table.table[i], sizeof(struct FAT32DirectoryEntry));
                found = true;
                break;
            }
        }
        if(!found) return 1;
    }
    return 0;
}

uint8_t extract_args(char* line, char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH]){
    memset(args, 0, MAX_COMMAND_ARGS * MAX_ARGS_LENGTH);

    // pointers
    uint16_t i = 0;

    // Skip first spaces
    while(line[i] == ' ') i++;

    uint16_t curr_char = 0;
    uint16_t curr_arg = 0;

    uint16_t last_curr_char = 0;
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
    uint16_t j = last_curr_char;
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

void handle_cd(char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH], struct DirectoryStack* dir_stack) {
    char folderName[DIR_NAME_LENGTH];
    if(args[1][0] == '\0'){
        puts("\nPlease provide folder name\n");
        return;
    }

    struct DirectoryStack input_path;
    input_path.length = 0;
    if (path_to_dir_stack(args[1], &input_path) != 0) return 1;
    
    uint8_t j;
    for (j = 0; j < input_path.length; j++)
    {
        if(memcmp(&input_path.entry[j].name, "..", 2) == 0) {
            if(memcmp(last_dir(dir_stack), "root", 4) == 0) {
                puts("\n");
                puts("Error: cannot pop root directory\n");
                return;
            } else {
                pop_dir(dir_stack);
            }
        }
    
        else {
            struct FAT32DirectoryTable dir_table;
            bool found = 0;
            get_dir(last_dir(dir_stack), prev_parent_cluster(dir_stack), &dir_table);
            for(uint32_t i = 2; i < 64; i++){
                if(is_empty(&dir_table.table[i])) continue;
                if(memcmp(dir_table.table[i].name, &input_path.entry[j].name, DIR_NAME_LENGTH) == 0){
                    if(is_directory(&dir_table.table[i])) {
                        push_dir(dir_stack, &dir_table.table[i]);
                    } else {
                        puts("\n");
                        puts(input_path.entry[j].name);
                        puts(" is not a folder.");
                        return;
                    }
                    found = 1;
                    break;
                }
            }
            if (found) continue ;
            else ;
                puts("\n");
                puts("Folder ");
                puts(input_path.entry[j].name);
                puts(" not found");
                return;
        }
    }
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

// if -1 then not found
uint8_t get_parent_cluster_in_parent_cluster(char folderName[DIR_NAME_LENGTH], uint8_t parent_cluster_to_find){
    struct FAT32DirectoryTable dir_table;
    get_dir(folderName, parent_cluster_to_find, &dir_table);
    for(uint32_t i = 2; i < 64; i++){
        if(is_empty(&dir_table.table[i])) continue;
        if(memcmp(&dir_table.table[i].name, folderName, DIR_NAME_LENGTH) == 0){
            return dir_table.table[i].cluster_low;
        }
    }
    return -1;
}

void handle_mkdir(char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH], struct DirectoryStack* dir_stack) {
    if(args[1][0] == '\0'){
        puts("\nPlease provide folder name\n");
        return;
    }
    struct DirectoryStack input_path = {.length = 0};
    uint8_t error_code = path_to_dir_stack(args[1], &input_path);
    if(error_code != 0) {
        puts("\nInvalid path\n");
        return;
    }

    for(uint8_t i = 0; i < input_path.length; i++) {
        if(memcmp(input_path.entry[i].name, "..", 2) == 0) {
            puts("\nCreating folder with name isn't allowed '..'\n");
            return;
        }
    }

    uint8_t idx = 0;
    uint16_t parent_cluster = current_parent_cluster(dir_stack);
    for(idx = 0; idx < input_path.length; idx++) {
        uint8_t error_code = make_directory(input_path.entry[idx].name, parent_cluster);

        struct FAT32DirectoryTable dir_table;
        get_dir(input_path.entry[idx].name, parent_cluster, &dir_table);
        parent_cluster = dir_table.table[0].cluster_low;
        get_dir("..\0\0\0\0\0\0", parent_cluster, &dir_table);


        for(uint32_t i = 2; i < 64; i++){
            if(is_empty(&dir_table.table[i])) continue;
            if(memcmp(&dir_table.table[i].name, input_path.entry[idx].name, DIR_NAME_LENGTH) == 0){
                parent_cluster = dir_table.table[i].cluster_low;
                break; 
            }
        }
        if(error_code == 1) continue; // folder already exist so its fine

        if(parent_cluster == -1) {
            puts_color("\nError: invalid parent cluster", Color_Red, Color_Black);
            return;
        }
        if(error_code != 0) break;
    }
    if(error_code == 1) {
        puts("\nFolder ");
        puts(input_path.entry[idx].name);
        puts(" already exist");

        // Delete all the folders that have been created
        for(uint8_t i = 0; i < idx; i++) {
            delete_file_or_dir(&input_path.entry[i], current_parent_cluster(dir_stack));
        }
        return;
    }
}

void handle_cat(char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH], struct DirectoryStack* dir_stack){
    char fileName[DIR_NAME_LENGTH];
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

void handle_rm(char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH], struct DirectoryStack* dir_stack){
    char fileName[DIR_NAME_LENGTH];
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

void handle_cp(char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH], struct DirectoryStack* dir_stack){
    char* src = args[0];
    char* dest = args[1];

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

void handle_mv(char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH], struct DirectoryStack* dir_stack){
    char src[MAX_ARGS_LENGTH];
    char dest[MAX_ARGS_LENGTH];

    memcpy(src, args[1], MAX_ARGS_LENGTH);
    memcpy(dest, args[2], MAX_ARGS_LENGTH);
}


// each folder wont contain more than 1 file/folder with same name.
void recursive_find(struct FAT32DirectoryTable* dir_table, char file_name[MAX_ARGS_LENGTH], uint32_t cluster_number, struct DirectoryStack* dir_stack){ // depth is just for debugging

    for(uint32_t i = 2; i < 64; i++){
        if(is_empty(&dir_table->table[i])) continue;
        
        
        if(memcmp(dir_table->table[i].name, file_name, DIR_NAME_LENGTH) == 0 /* && memcmp(dir_table.table[i].name, file_name, DIR_EXT_LENGTH) == 0 */ ){
            // traversal back to populate dir_stack 
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
            print_path_to_cwd_reversed(&list);
            puts("\n");
        }

        if(is_directory(&dir_table->table[i])) {
            struct FAT32DirectoryTable children;
            get_dir(dir_table->table[i].name, cluster_number, &children);
            recursive_find(&children, file_name, dir_table->table[i].cluster_low, dir_stack);
        }


        // print from root for debugging
        // else {
        //     uint16_t parent_cluster = cluster_number;
        //     struct FAT32DirectoryTable children;
        //     struct DirectoryStack list = {.length = 0};
        //     push_dir(&list, &dir_table->table[i]);
        //     push_dir(&list, &dir_table->table[0]);
            
        //     while(parent_cluster != ROOT_CLUSTER_NUMBER){
        //         get_dir("..\0\0\0\0\0\0", parent_cluster, &children);
        //         parent_cluster = children.table[1].cluster_low;
        //         push_dir(&list, &children.table[0]);
        //     }
        //     print_path_to_cwd(&list);
        //     puts("\n");
        // }
    }
}

void handle_find(char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH]){
    char file_name[MAX_ARGS_LENGTH]; // TODO: validate if this is not a path
    memcpy(file_name, args[1], MAX_ARGS_LENGTH);
    
    struct DirectoryStack dir_stack[100];
    struct FAT32DirectoryTable dir_table;
    get_dir("root\0\0\0\0", ROOT_CLUSTER_NUMBER, &dir_table);
    puts("\n");
    recursive_find(&dir_table, file_name, ROOT_CLUSTER_NUMBER, dir_stack);

}

const char clear[6] = "clear\0";
const char help[5] = "help\0";
const char cd[3] = "cd\0"; // cd	- Mengganti current working directory (termasuk .. untuk naik)
const char ls[3] = "ls\0"; // ls	- Menuliskan isi current working directory
const char mkdir[6] = "mkdir\0"; // mkdir	- Membuat sebuah folder kosong baru
const char cat[4] = "cat\0"; // cat	- Menuliskan sebuah file sebagai text file ke layar (Gunakan format LF newline)
const char cp[3] = "cp\0"; // cp	- Mengcopy suatu file (Folder menjadi bonus)
const char rm[3] = "rm\0"; // rm	- Menghapus suatu file (Folder menjadi bonus)
const char mv[3] = "mv\0"; // mv	- Memindah dan merename lokasi file/folder
const char find[5] = "find\0"; // find	- Mencari file/folder dengan nama yang sama diseluruh file system

void command(char *buf, struct DirectoryStack* dir_stack) {
    
    char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH];
    extract_args(buf, args);

    if(strcmp(args[0], clear) == 0) {
        clear_screen();
        set_cursor(0, 0);
        return; // prevent new line
    } else if (strcmp(args[0], cd) == 0) {
        handle_cd(args, dir_stack);
    } else if (strcmp(args[0], ls) == 0) {
        handle_ls(dir_stack);
    } else if (strcmp(args[0], mkdir) == 0) {
        handle_mkdir(args, dir_stack);
    } else if (strcmp(args[0], cat) == 0) {
        handle_cat(args, dir_stack);
    } else if (strcmp(args[0], cp) == 0) {
        handle_cp(args, dir_stack);
    } else if (strcmp(args[0], rm) == 0) {
        handle_rm(args, dir_stack);
    } else if (strcmp(args[0], mv) == 0) {
        handle_mv(args, dir_stack);
    } else if (strcmp(args[0], find) == 0) {
        handle_find(args);
    } else if (strcmp(args[0], help) == 0) {
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