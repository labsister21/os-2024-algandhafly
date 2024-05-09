#include <io.h>
#include <command.h>
#include <system.h>
#include <cwdlist.h>

#define MAX_COMMAND_LENGTH 2000


int main(void) {
    char buf[MAX_COMMAND_LENGTH];
    struct CWDList cwd_list; push_dir(&cwd_list, "root\0\0\0\0", 2);

    while(true) {
        print_cwd(&cwd_list);
        get_line(buf);
        command(buf, &cwd_list);
        puts("\n");
    }


    return 0;
}
