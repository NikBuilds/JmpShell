#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <linux/limits.h>
#include <dirent.h>
#include <sys/stat.h>

#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\x1B[0m"

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



typedef struct {
    char **command_line;
    int count;
} Command;

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
                             (cmd_struct.count + 1) * sizeof(char*));

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



typedef struct {
    char **sub_folders;
    char *root_path;
    int count;
} Directory;

int create_dir(Directory *subf) 
{
    subf->sub_folders = NULL;
    subf->root_path = NULL;
    subf->count = 0; 

    DIR *dir = opendir("."); // Opens current directory

    if (!dir) {
        perror("opendir");
        return 1;
    }

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) perror("Faild to get the current directory");

    subf->root_path = malloc(strlen(cwd));
    strcpy(subf->root_path, cwd);

    struct dirent *entry;       // Struct for every entry in the folder
    struct stat st;             // Struct to save data about the Files or folders

    // readdir() reads every entry in the current folder and saves the current entry trough every iteration into *entry
    while ((entry = readdir(dir)) != NULL) { 

        // Skip . (Current dir) and .. (root dir)
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // stat() retrieves information from the operating system about the passed entry and saves this data in st
        // S_ISDIR(st.st_mode) checks if the entry is a directory
        if (stat(entry->d_name, &st) == 0 && S_ISDIR(st.st_mode)) {
            char **temp = realloc(subf->sub_folders,  
                (subf->count + 1) * sizeof(char*));

            if(temp == NULL) {
                perror("Realloc failed");
                break;
            }
            
            // Assign memory address
            subf->sub_folders = temp;

            subf->sub_folders[subf->count] = malloc(strlen(entry->d_name) + 1); // + 1 for Nullbyte '\0'
            if (subf->sub_folders[subf->count] == NULL) {
                perror("malloc failed");
                break;
            }

            strcpy(subf->sub_folders[subf->count], entry->d_name);
            subf->count++; 
        }
    }

    closedir(dir);
    return 0;
}

void free_dir(Directory *dir) 
{
    for (int i = 0; i < dir->count; i++) {
        free(dir->sub_folders[i]);
    }
    free(dir->sub_folders);
}



int main() {
    Directory dir;
    create_dir(&dir);
        
    while(1) {
        printf("%s[JumpShell] ", KRED);
        printf("%s%s\n", KCYN, dir.root_path);
        printf(KCYN "<" KWHT "./" KCYN "> " RESET);

        char *line = read_line();
        Command cmd = split_command(line);

        if(!strcmp("exit", cmd.command_line[0])) {
            free_command(&cmd);
            printf("Goodbye jumper!\n");
            break;
        } 

        if(!strcmp("subf", cmd.command_line[0])) {
            for(int i = 0; i < dir.count; i++) {
                printf("/%s\n", dir.sub_folders[i]);
            }
            
            free_command(&cmd);
            free(line);
            continue;
        }

        // Create child process
        int status;
        pid_t child_pid = fork();

        if(child_pid < 0 ) {
            perror("fork fail");
            exit(1);
        }

        if(child_pid == 0){
            // Replace child process with the programm
            // If success, execvp() never returns!
            execvp(cmd.command_line[0], cmd.command_line);
            
            perror("execvp fail");
            _exit(EXIT_FAILURE); // Kills process instantly. Important because if the child fails, the buffe could flush etc. 
        } 

        waitpid(child_pid, &status, 0);

        free_command(&cmd);
        free(line);
    }

    free_dir(&dir);
    return 0;
}