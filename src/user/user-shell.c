#include <io.h>
#include <command.h>
#include <system.h>
#include <cwdlist.h>

#define MAX_COMMAND_LENGTH 100


int main(void) {
    init_user_driver_state();
    char buf[MAX_COMMAND_LENGTH];
    struct CWDList cwd_list; append_dir(&cwd_list, "root\0\0\0\0");

    while(true) {
        print_cwd(&cwd_list);
        get_line(buf);
        command(buf, &cwd_list);
        puts("\n");
    }


    return 0;
}
