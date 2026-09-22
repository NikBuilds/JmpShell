#include "jmpShell.h"

int main() {
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

    struct termios orig_termio;
    char* line = NULL;

    Directory dir;
    create_dir(&dir);
    enableRawMode(&orig_termio); 
    
    while(1) {
        printf("%s[JumpShell] %s%s%s\r\n", KRED, KCYN, dir.root_path, RESET);
        printf("> ");
        fflush(stdout);
        
        shellProcessKeypress(&line);
        Command cmd = split_command(line);
    }

    disableRawMode(&orig_termio);
    return 0;
}