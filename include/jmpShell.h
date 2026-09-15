#ifndef JMPSHELL_H
#define JMPSHELL_H

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

// cmd
typedef struct {
    char **command_line;
    int count;
} Command;

char* read_line();
Command split_command(char *line);
void free_command(Command *cmd);

// dir
typedef struct {
    char **sub_folders;
    char *root_path;
    int count;
} Directory;

int create_dir(Directory *subf);
void free_dir(Directory *dir);

#endif