#include <command.h>
#include <directorystack.h>
#include <system.h>
#include <io.h>
#include <string.h>
#include <time.h>
#include "header/filesystem/fat32.h"
#include "header/process/process.h"


#define MAX_COMMAND_ARGS 20
#define MAX_ARGS_LENGTH 200
#define not !
#define but &&
#define and &&
#define or ||
#define DOT '.'
#define FWSLASH '/'
#define NULL_CHAR '\0'

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
}

// dir_stack_cwd: the cwd passed from the user-shell
// dir_stack: the dir_stack to be populated
// return 1: not found, 2: last element not found
uint8_t path_to_dir_stack_from_cwd(char* path, struct DirectoryStack* dir_stack_cwd, struct DirectoryStack* dir_stack) {
    uint8_t error_code = path_to_dir_stack(path, dir_stack);
    if(error_code != 0) return error_code;

    uint8_t idx = 0;
    uint16_t parent_cluster = current_parent_cluster(dir_stack_cwd);
    for(idx = 0; idx < dir_stack->length; idx++) {

        struct FAT32DirectoryTable dir_table;
        get_dir_by_cluster(parent_cluster, &dir_table);

        bool found = false;
        for(uint32_t i = 0; i < 64; i++){
            if(is_empty(&dir_table.table[i])) continue;
            if(memcmp(&dir_table.table[i].name, dir_stack->entry[idx].name, DIR_NAME_LENGTH) == 0){
                parent_cluster = dir_table.table[i].cluster_low;
                memcpy(&dir_stack->entry[idx], &dir_table.table[i], sizeof(struct FAT32DirectoryEntry));
                found = true;
                break; 
            }
        }

        // case last element and is a file
        if(!found && idx == dir_stack->length - 1) {
            return 2;
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
    bool is_quoted = false;
    while(line[i] != '\0') {
        if(line[i] == '"') {
            is_quoted = !is_quoted;
            i++;
            continue;
        }
        if(is_quoted) {
            args[curr_arg][curr_char] = line[i];
            curr_char++;
            last_curr_char = curr_char;
            i++;
            continue;
        }

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

int handle_cd(char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH], struct DirectoryStack* dir_stack) {
    char folderName[DIR_NAME_LENGTH];
    if(args[1][0] == '\0'){
        puts("\nPlease provide folder name\n");
        return 0;
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
                return 0;
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
                        return 0;
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
                return 0;
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
            if(dir_table.table[i].ext[0] != NULL_CHAR || dir_table.table[i].ext[1] != NULL_CHAR || dir_table.table[i].ext[2] != NULL_CHAR) {
                puts_color(".", Color_LightBlue, Color_Black);
                puts_color(dir_table.table[i].ext, Color_LightBlue, Color_Black);
            }
        }
        puts("\n");
    }
    if(!has_any){
        puts("Empty folder");
    }
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
        get_dir_by_cluster(parent_cluster, &dir_table);
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
    if(args[1][0] == '\0'){
        puts("\nPlease provide folder name\n");
        return;
    }
    struct DirectoryStack input_path = {.length = 0};
    uint8_t error_code = path_to_dir_stack_from_cwd(args[1], dir_stack, &input_path);
    if(error_code != 0) {
        puts("\nInvalid path\n");
        return;
    }

    char* file_name = peek_top(&input_path)->name;
    char* ext = peek_top(&input_path)->ext;
    uint16_t parent_cluster_containing_file;
    if(input_path.length == 1)parent_cluster_containing_file = current_parent_cluster(dir_stack);
    else parent_cluster_containing_file = peek_second_top(&input_path)->cluster_low;


    struct FAT32DirectoryEntry entry;
    memcpy(entry.name, file_name, DIR_NAME_LENGTH); // kano
    memcpy(entry.ext, ext, DIR_EXT_LENGTH);
    uint32_t content_size = 2048;

    error_code;
    while(true) {
        entry.filesize = content_size;
        char content[content_size];
        // Error code: 0 success - 1 not a file - 2 not enough buffer - 3 not found - -1 unknown
        error_code = read_file(&entry, parent_cluster_containing_file, content);
        if(error_code == 1) {
            puts("\n");
            puts(file_name);
            puts(ext);
            puts(" is not a file");
            return;
        } else if(error_code == 2) {
            content_size += 2048;
            continue;
        } else if(error_code == 3) {
            puts("\n");
            puts(file_name);
            puts(ext);
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
    if(args[1][0] == '\0'){
        puts("\nPlease provide folder name\n");
        return;
    }
    struct DirectoryStack input_path = {.length = 0};
    uint8_t error_code = path_to_dir_stack_from_cwd(args[1], dir_stack, &input_path);
    if(error_code != 0) {
        puts("\nInvalid path\n");
        return;
    }
    char* file_name = peek_top(&input_path)->name;
    char* ext = peek_top(&input_path)->ext;
    uint16_t parent_cluster_containing_file;
    if(input_path.length == 1)parent_cluster_containing_file = current_parent_cluster(dir_stack);
    else parent_cluster_containing_file = peek_second_top(&input_path)->cluster_low;

    struct FAT32DirectoryEntry entry = {
        .filesize = 0xFFFF,
    };

    memcpy(entry.name, file_name, DIR_NAME_LENGTH);
    memcpy(entry.ext, ext, DIR_EXT_LENGTH);

    error_code = delete_file_or_dir(&entry, parent_cluster_containing_file);
    // Error code: 0 success - 1 not found - 2 folder is not empty - -1 unknown
    if(error_code == 0) {
        // used in mv
        // puts("\nDeleted ");
        // puts(entry.name);
        // puts("\n");
    } else if(error_code == 1){
        puts("\nFolder not found\n");
    } else if(error_code == 2){
        puts("\nFolder is not empty\n");
    } else if(error_code == -1){
        puts("\nUnknown error has occured\n");
    }
}

void handle_cp(char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH], struct DirectoryStack* dir_stack){
    char* src = args[1];
    char* dest = args[2];

    if(src[0] == '\0'){
        puts("\nPlease provide source file name\n");
        return;
    }
    if(dest[0] == '\0'){
        puts("\nPlease provide destination file name\n");
        return;
    }

    struct DirectoryStack src_path = {.length = 0};
    uint8_t error_code = path_to_dir_stack_from_cwd(src, dir_stack, &src_path);
    if(error_code != 0) {
        puts("\nInvalid source path\n");
        return;
    }
    struct DirectoryStack dest_path = {.length = 0};
    error_code = path_to_dir_stack_from_cwd(dest, dir_stack, &dest_path);
    if(error_code == 1) {
        puts("\nInvalid destination path\n");
        return;
    }

    uint16_t parent_cluster_containing_src_file;
    if(src_path.length == 1)parent_cluster_containing_src_file = current_parent_cluster(dir_stack);
    else parent_cluster_containing_src_file = peek_second_top(&src_path)->cluster_low;

    uint16_t parent_cluster_containing_dest_file;
    if(dest_path.length == 1)parent_cluster_containing_dest_file = current_parent_cluster(dir_stack);
    else parent_cluster_containing_dest_file = peek_second_top(&dest_path)->cluster_low;

    struct FAT32DirectoryEntry entry;
    memcpy(entry.name, peek_top(&src_path)->name, DIR_NAME_LENGTH);
    memcpy(entry.ext, peek_top(&src_path)->ext, DIR_EXT_LENGTH);
    uint32_t content_size = 2048;


    // Read src
    while(true) {
        entry.filesize = content_size;
        char content[content_size];
        error_code = read_file(&entry, parent_cluster_containing_src_file, content);
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
        
        memcpy(copy.name, peek_top(&dest_path)->name, DIR_NAME_LENGTH);
        memcpy(copy.ext, peek_top(&dest_path)->ext, DIR_EXT_LENGTH);
        write_file(&copy, parent_cluster_containing_dest_file, content);

        break;
    }


}

void handle_mv(char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH], struct DirectoryStack* dir_stack){
    handle_cp(args, dir_stack);
    handle_rm(args, dir_stack);
}


// each folder wont contain more than 1 file/folder with same name.
void recursive_find(struct FAT32DirectoryTable* dir_table, char file_name[MAX_ARGS_LENGTH], uint32_t cluster_number, struct DirectoryStack* dir_stack){ // depth is just for debugging

    for(uint32_t i = 2; i < 64; i++){
        if(is_empty(&dir_table->table[i])) continue;
        
        
        if(memcmp(dir_table->table[i].name, file_name, DIR_NAME_LENGTH) == 0 /* && memcmp(dir_table.table[i].name, file_name, DIR_EXT_LENGTH) == 0 */ ){
            // traversal back to populate dir_stack 
            uint16_t parent_cluster = cluster_number;
            struct FAT32DirectoryTable parents;
            struct DirectoryStack list = {.length = 0};
            push_dir(&list, &dir_table->table[i]);
            
            while(parent_cluster != ROOT_CLUSTER_NUMBER){
                get_dir_by_cluster(parent_cluster, &parents);
                parent_cluster = parents.table[1].cluster_low;
                push_dir(&list, &parents.table[0]);
            }
            push_dir(&list, &dir_table->table[0]);
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

void handle_echo(char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH], struct DirectoryStack* dir_stack) {
    char* file_content = args[1];
    char* right = args[2];
    char* dest = args[3];

    if(file_content[0] == '\0'){
        puts("\nPlease provide content\n");
        return;
    }
    if(right[0] != '>'){
        puts("\n");
        puts(file_content);
        return;
    }
    if(dest[0] == '\0'){
        puts("\nPlease provide destination file path\n");
        return;
    }

    struct DirectoryStack dest_path = {.length = 0};
    uint8_t error_code  = path_to_dir_stack_from_cwd(dest, dir_stack, &dest_path);
    if(error_code == 1) {
        puts("\nInvalid destination path\n");
        return;
    }

    uint16_t parent_cluster_containing_dest_file;
    if(dest_path.length == 1)parent_cluster_containing_dest_file = current_parent_cluster(dir_stack);
    else parent_cluster_containing_dest_file = peek_second_top(&dest_path)->cluster_low;

    struct FAT32DirectoryEntry entry;
    memcpy(entry.name, peek_top(&dest_path)->name, DIR_NAME_LENGTH);
    memcpy(entry.ext, peek_top(&dest_path)->ext, DIR_EXT_LENGTH);
    entry.filesize = strlen(file_content);

    write_file(&entry, parent_cluster_containing_dest_file, file_content);
    puts("\n");
    
}


void handle_exec(char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH], struct DirectoryStack* dir_stack){
    char* path = args[1];
    struct DirectoryStack file_path;
    uint8_t error_code = path_to_dir_stack_from_cwd(path, dir_stack, &file_path);
    if(error_code == 1) {
        puts("\nInvalid path\n"); return;
    }
    if(error_code == 2) {
        puts("\nFile don't exist\n"); return;
    }

    if(is_directory(peek_top(&file_path))) {
        puts("\n"); puts(path); puts(" is not a file\n"); return;
    }

    uint16_t parent_cluster_containing_file;
    if(file_path.length == 1)parent_cluster_containing_file = current_parent_cluster(dir_stack);
    else parent_cluster_containing_file = peek_second_top(&file_path)->cluster_low;

    struct FAT32DirectoryEntry entry;
    memcpy(entry.name, peek_top(&file_path)->name, DIR_NAME_LENGTH);
    memcpy(entry.ext, peek_top(&file_path)->ext, DIR_EXT_LENGTH);

    execute_file(&entry, parent_cluster_containing_file);
}


void handle_ps(char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH], struct DirectoryStack* dir_stack){
    systemCall(12, 0, 0, 0);
    puts("\n");
}
void handle_kill(char args[MAX_COMMAND_ARGS][MAX_ARGS_LENGTH], struct DirectoryStack* dir_stack) {
    systemCall(13, 0, args[1], 0);
}
void handle_clock(){
    puts("\nClock running...\n");
    activate_clock_screen();
}

void handle_exit(){
    puts("\nExiting...\n");
    exit_user_shell();
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
const char echo[5] = "echo\0"; // echo - can be used to write to file

const char exec[5] = "exec\0";
const char ps[3] = "ps\0";
const char kill[5] = "kill\0";
const char clock[6] = "clock\0";

const char exit[5] = "exit\0";

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
    } else if (strcmp(args[0], echo) == 0) {
        handle_echo(args, dir_stack);
    } else if (strcmp(args[0], exec) == 0) {
        handle_exec(args, dir_stack);
    } else if (strcmp(args[0], ps) == 0) {
        handle_ps(args, dir_stack);
    } else if (strcmp(args[0], kill) == 0) {
        handle_kill(args, dir_stack);
    } else if (strcmp(args[0], clock) == 0) {
        handle_clock();
    } else if (strcmp(args[0], exit) == 0) {
        handle_exit();
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