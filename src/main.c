#include "jmpShell.h"

int main()
{
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
    char *line = NULL;

    enableRawMode(&orig_termio);

    while (1)
    {
        Directory dir;
        create_dir(&dir);

        // root_path ist NULL, wenn create_dir() fehlschlägt
        printf("%s[JumpShell] %s%s%s\r\n", KRED, KCYN, dir.root_path ? dir.root_path : "?", RESET);
        printf("> ");
        fflush(stdout);

        shellProcessKeypress(&line);
        if (line == NULL || line[0] == '\0')
        {
            free_dir(&dir);
            continue;
        }

        Command cmd = split_command(line);
        if (cmd.count == 0) // z. B. Eingabe nur aus Leerzeichen
        {
            free_command(&cmd);
            free_dir(&dir);
            continue;
        }

        // ----- Test -----
        for (int i = 0; i < cmd.count; i++)
        {
            printf("%s\r\n", cmd.command_line[i]);
        }
        printf("%s\r\n", cmd.command_line[cmd.count] == NULL ? "NULL" : "NOT NULL!");
        printf("\r\n");
        // ----------------

        if (!strcmp("exit", cmd.command_line[0]))
        {
            free_command(&cmd);
            free_dir(&dir);
            printf("Goodbye jumper!\r\n");
            break;
        }

        if (!strcmp("subf", cmd.command_line[0]))
        {
            printf("Call subf!\r\n");
            for (int i = 0; i < dir.count; i++)
            {
                printf("[%i] ./%s\r\n", i, dir.sub_folders[i]);
            }

            free_dir(&dir);
            free_command(&cmd);
            continue;
        }

        if (!strcmp("cd", cmd.command_line[0]))
        {
            if (cmd.count < 2)
            {
                fprintf(stderr, "cd: missing argument\r\n");
            }
            else if (chdir(cmd.command_line[1]) != 0)
            {
                // Kein perror() da diese funktion nach stderr immer mit \n schreibt.
                // Somit kommt kein Carriage Return und Treppeneffekt tritt ein.
                // Deshalb wird der Fehlertest mit strerror() selbst ausgegeben.
                fprintf(stderr, "%s: %s\r\n", cmd.command_line[1], strerror(errno));
            }

            free_command(&cmd);
            free_dir(&dir);
            continue;
        }

        // Create child process
        int status;
        pid_t child_pid = fork();

        if (child_pid < 0)
        {
            perror("fork fail");
            exit(1);
        }

        if (child_pid == 0)
        {
            // Replace child process with the programm
            // If success, execvp() never returns!
            execvp(cmd.command_line[0], cmd.command_line);

            perror("execvp fail");
            _exit(EXIT_FAILURE); // Kills process instantly. Important because if the child fails, the buffe could flush etc.
        }

        // Parent wait till child is finished
        waitpid(child_pid, &status, 0);

        // Already built in in shellProcessKeaypress but doesent hurt.
        free(line);
        line = NULL;
    }

    free(line);
    disableRawMode(&orig_termio);
    return 0;
}