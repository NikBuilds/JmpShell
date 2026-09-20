#include "jmpShell.h"

static int run_once = 0;

void print_headline(const char *root_path) {
        if(run_once == 0) {
            printf(KMAG
                "  ▄▄▄▄▄▄               ▄▄▄▄▄              ▄▄ ▄▄ \n"
                " █▀ ██                ██▀▀▀▀█▄ █▄          ██ ██\n"
                "    ██ ▄              ▀██▄  ▄▀ ██          ██ ██\n"
                "    ██ ███▄███▄ ████▄   ▀██▄▄  ████▄ ▄█▀█▄ ██ ██\n"
                "    ██ ██ ██ ██ ██ ██ ▄   ▀██▄ ██ ██ ██▄█▀ ██ ██\n"
                "    ██▄██ ██ ▀█▄████▀ ▀██████▀▄██ ██▄▀█▄▄▄▄██▄██\n"
                "▄   ██          ██                              \n"
                "▀████▀          ▀                               \n"
                "\n" RESET);
        }

        printf("%s[JumpShell] ", KRED);
        printf("%s%s\n", KCYN, root_path);
        printf(KCYN "<" KWHT "./" KCYN "> " RESET);
        if (run_once == 0) run_once++;
}

int main() {
    enableRawMode(); 

    shellProcessKeypress();
    


    return 0;
}