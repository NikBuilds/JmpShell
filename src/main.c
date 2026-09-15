#include "jmpShell.h"

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
        
    while(1) {
        create_dir(&dir);

        printf("%s[JumpShell] ", KRED);
        printf("%s%s\n", KCYN, dir.root_path);
        printf(KCYN "<" KWHT "./" KCYN "> " RESET);

        char *line = read_line();
        Command cmd = split_command(line);

        if(!strcmp("exit", cmd.command_line[0])) 
        {
            free_command(&cmd);
            printf("Goodbye jumper!\n");
            break;
        } 

        if(!strcmp("subf", cmd.command_line[0])) 
        {
            for(int i = 0; i < dir.count; i++) {
                printf("[%i] ./%s\n", i, dir.sub_folders[i]);
            }
            
            free_command(&cmd);
            free(line);
            continue;
        }

        if (!strcmp("cd", cmd.command_line[0])) 
        {
            if (cmd.count < 2) {
                fprintf(stderr, "cd: missing argument\n");
            }
            else if (chdir(cmd.command_line[1]) != 0) {
                perror("cd failed!");
            }

            free_command(&cmd);
            free(line);
            free_dir(&dir);
            continue;
        }


        // ----- Test -----
        // for(int i = 0; i < cmd.count; i++) {
        //    printf("%s\n", cmd.command_line[i]);

        // }
        // printf("%s\n", cmd.command_line[cmd.count] == NULL ? "NULL" : "NOT NULL!");
        // printf("\n");


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

        // Parent wait till child is finished
        waitpid(child_pid, &status, 0);

        free_command(&cmd);
        free(line);
        free_dir(&dir);
    }

    free_dir(&dir);
    return 0;
}