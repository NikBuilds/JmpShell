#include "jmpShell.h"

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

    subf->root_path = malloc(strlen(cwd) + 1 );

    if (subf->root_path == NULL) {
        perror("malloc failed");
        closedir(dir);
        return 1;
    }

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