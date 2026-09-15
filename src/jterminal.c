#include "jmpShell.h"

char* read_line() 
{
    char *line = NULL;
    char *tmp = NULL;
    size_t size = 0, index = 0;
    int ch = EOF;

    while (ch) {
        ch = getc(stdin);                       // Reads one char from standard input stdin 
        if(ch == EOF || ch == '\n') ch = 0;     // 0 = false so loop stops

        if(size <= index) {
            size += 1;
            tmp = realloc(line, size); 

            if(!tmp) {
                free(line);
                line = NULL;
                break;
            }
            line = tmp;
        }

        /* Store Chars into string. */
        line[index++] = ch;
    }
    return line;
}

// Hier realloc 2d array
Command split_command(char *line) 
{
    Command cmd_struct;
    cmd_struct.command_line = NULL;
    cmd_struct.count = 0;
    char *saveptr;          // Pos for token
    
    char *token = strtok_r(line, " ", &saveptr); // Splits string into tokens unsing " " as delimiter

    while (token != NULL) { // Loops trough the tokens
        char **temp = realloc(cmd_struct.command_line, 
                             (cmd_struct.count + 2) * sizeof(char*)); // +2 because need space for NULL

        if(temp == NULL) {
            perror("Realloc failed");
            break;
        }

        cmd_struct.command_line = temp;

        // Speicher für das Wort selbst reservieren
        cmd_struct.command_line[cmd_struct.count] = malloc(strlen(token) + 1); // + 1 for Nullbyte '\0'
        if (cmd_struct.command_line[cmd_struct.count] == NULL) {
            perror("malloc failed");
            break;
        }

        // Copy token into command_line
        strcpy(cmd_struct.command_line[cmd_struct.count], token);
        cmd_struct.count++;
        cmd_struct.command_line[cmd_struct.count] = NULL; // execvp() needs a NULL terminated array to know when the arguments end

        token = strtok_r(NULL, " ", &saveptr);
    }

    return cmd_struct;
}

void free_command(Command *cmd) 
{
    for (int i = 0; i < cmd->count; i++) {
        free(cmd->command_line[i]);
    }
    free(cmd->command_line);
}