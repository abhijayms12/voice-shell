#include "commands.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <direct.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

// ===================== ls (same logic, Windows APIs) =====================

void my_ls(const char *path) {
    WIN32_FIND_DATAA de;
    HANDLE d;
    struct stat buf;
    char timebuf[26];

    if (path == NULL || strlen(path) == 0)
        path = ".";

    char searchPath[MAX_PATH];
    snprintf(searchPath, MAX_PATH, "%s\\*", path);

    d = FindFirstFileA(searchPath, &de);
    if (d == INVALID_HANDLE_VALUE) {
        printf("Error: Cannot access '%s'\r\n", path);
        return;
    }

    char oldcwd[MAX_PATH];
    _getcwd(oldcwd, sizeof(oldcwd));
    _chdir(path);

    printf("Name                           Type       Size        Modified\r\n");
    printf("------------------------------------------------------------------------\r\n");

    do {
        /* Skip current and parent directory entries */
        if (strcmp(de.cFileName, ".") == 0 || strcmp(de.cFileName, "..") == 0)
            continue;

        if (stat(de.cFileName, &buf) != 0)
            continue;

        /* Modification time */
        struct tm *t = localtime(&buf.st_mtime);
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", t);

        /* Print each file/folder on its own line */
        if (S_ISDIR(buf.st_mode)) {
            printf("%-30s <DIR>      %-11s %s\r\n", de.cFileName, "", timebuf);
        } else {
            printf("%-30s FILE       %-11ld %s\r\n", de.cFileName, (long)buf.st_size, timebuf);
        }

    } while (FindNextFileA(d, &de));

    FindClose(d);
    _chdir(oldcwd);
}

// ===================== pwd =====================

void my_pwd() {
    char cwd[MAX_PATH];
    if (_getcwd(cwd, sizeof(cwd)) != NULL)
        printf("%s\r\n", cwd);
    else
        perror("pwd");
}

// ===================== cd =====================

void my_cd(const char *path) {
    if (!path || strlen(path) == 0) {
        const char *home = getenv("USERPROFILE");
        if (!home) home = "C:\\";
        if (_chdir(home) != 0) {
            printf("Error: Cannot change to home directory\r\n");
        } else {
            printf("Changed to: %s\r\n", home);
        }
        return;
    }

    if (_chdir(path) != 0) {
        printf("Error: Cannot access '%s'\r\n", path);
    } else {
        char cwd[MAX_PATH];
        _getcwd(cwd, sizeof(cwd));
        printf("Changed to: %s\r\n", cwd);
    }
}

// ===================== cat =====================

void my_cat(const char *filename) {
    if (!filename) {
        printf("Error: cat requires a filename\r\n");
        return;
    }

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("Error: Cannot open file '%s'\r\n", filename);
        return;
    }

    char buffer[4096];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0)
        fwrite(buffer, 1, n, stdout);

    fclose(fp);
}

// ===================== mkdir =====================

void my_mkdir(const char *dirname) {
    if (!dirname || strlen(dirname) == 0) {
        printf("Error: mkdir requires a directory name\r\n");
        return;
    }

    if (_mkdir(dirname) != 0) {
        printf("Error: Cannot create directory '%s'\r\n", dirname);
    } else {
        printf("Directory created: %s\r\n", dirname);
    }
}

// ===================== rmdir =====================

void my_rmdir(const char *dirname) {
    if (!dirname || strlen(dirname) == 0) {
        printf("Error: rmdir requires a directory name\r\n");
        return;
    }

    if (_rmdir(dirname) != 0) {
        printf("Error: Cannot remove directory '%s'\r\n", dirname);
    } else {
        printf("Directory removed: %s\r\n", dirname);
    }
}

// ===================== touch =====================

void my_touch(const char *filename) {
    if (!filename || strlen(filename) == 0) {
        printf("Error: touch requires a filename\r\n");
        return;
    }

    HANDLE hFile = CreateFileA(
        filename,
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Error: Cannot create or update file '%s'\r\n", filename);
        return;
    }

    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    SetFileTime(hFile, NULL, NULL, &ft);
    CloseHandle(hFile);
    printf("File created/updated: %s\r\n", filename);
}

// ===================== rm =====================

void my_rm(const char *filename) {
    if (!filename || strlen(filename) == 0) {
        printf("Error: rm requires a filename\r\n");
        return;
    }

    if (remove(filename) != 0) {
        printf("Error: Cannot remove file '%s'\r\n", filename);
    } else {
        printf("File removed: %s\r\n", filename);
    }
}

// ===================== cp =====================

void my_cp(const char *src, const char *dest) {
    if (!src || !dest) {
        printf("Error: cp requires source and destination files\r\n");
        return;
    }

    FILE *in = fopen(src, "rb");
    if (!in) {
        printf("Error: Cannot open source file '%s'\r\n", src);
        return;
    }

    FILE *out = fopen(dest, "wb");
    if (!out) {
        printf("Error: Cannot create destination file '%s'\r\n", dest);
        fclose(in);
        return;
    }

    char buffer[8192];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0)
        fwrite(buffer, 1, n, out);

    fclose(in);
    fclose(out);
    printf("Copied '%s' to '%s'\r\n", src, dest);
}

// ===================== mv =====================

void my_mv(const char *src, const char *dest) {
    if (!src || !dest) {
        printf("Error: mv requires source and destination\r\n");
        return;
    }

    if (rename(src, dest) != 0) {
        printf("Error: Cannot move/rename '%s' to '%s'\r\n", src, dest);
    } else {
        printf("Moved/renamed '%s' to '%s'\r\n", src, dest);
    }
}

// ===================== help =====================

void help() {
    printf("Voice Enabled Command Line Interpreter - Available Commands:\r\n\r\n");
    printf("  ls [path]           - List files and directories\r\n");
    printf("  pwd                 - Print current working directory\r\n");
    printf("  cd [path]           - Change directory\r\n");
    printf("  cat <file>          - Display file contents\r\n");
    printf("  mkdir <dir>         - Create new directory\r\n");
    printf("  rmdir <dir>         - Remove empty directory\r\n");
    printf("  touch <file>        - Create or update file\r\n");
    printf("  rm <file>           - Remove file\r\n");
    printf("  cp <src> <dest>     - Copy file\r\n");
    printf("  mv <src> <dest>     - Move/rename file\r\n");
    printf("  help                - Show this help message\r\n");
    printf("  exit                - Close the application\r\n");
}

// ===================== GUI Integration Functions =====================

void get_current_directory(char *buffer, int size) {
    if (_getcwd(buffer, size) == NULL) {
        strncpy(buffer, ".", size - 1);
        buffer[size - 1] = '\0';
    }
}

void execute_command(const char *input, char *output) {
    if (!input || !output) return;
    
    /* Create temporary file for output capture */
    char tempFilename[MAX_PATH];
    GetTempPathA(MAX_PATH, tempFilename);
    strcat(tempFilename, "vsh_output.txt");
    
    /* Redirect stdout to temp file */
    FILE *old_stdout = stdout;
    freopen(tempFilename, "w", stdout);
    freopen(tempFilename, "w", stderr);
    
    /* Parse command and arguments */
    char cmd[1024];
    strncpy(cmd, input, sizeof(cmd) - 1);
    cmd[sizeof(cmd) - 1] = '\0';
    
    char *token = strtok(cmd, " \t");
    if (!token) {
        fclose(stdout);
        fclose(stderr);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        output[0] = '\0';
        DeleteFileA(tempFilename);
        return;
    }
    
    char *arg1 = strtok(NULL, " \t");
    char *arg2 = strtok(NULL, " \t");
    
    /* Execute command */
    if (strcmp(token, "ls") == 0) {
        my_ls(arg1);
    }
    else if (strcmp(token, "pwd") == 0) {
        my_pwd();
    }
    else if (strcmp(token, "cd") == 0) {
        my_cd(arg1);
    }
    else if (strcmp(token, "cat") == 0) {
        my_cat(arg1);
    }
    else if (strcmp(token, "mkdir") == 0) {
        my_mkdir(arg1);
    }
    else if (strcmp(token, "rmdir") == 0) {
        my_rmdir(arg1);
    }
    else if (strcmp(token, "touch") == 0) {
        my_touch(arg1);
    }
    else if (strcmp(token, "rm") == 0) {
        my_rm(arg1);
    }
    else if (strcmp(token, "cp") == 0) {
        my_cp(arg1, arg2);
    }
    else if (strcmp(token, "mv") == 0) {
        my_mv(arg1, arg2);
    }
    else if (strcmp(token, "help") == 0) {
        help();
    }
    else if (strcmp(token, "exit") == 0) {
        fclose(stdout);
        fclose(stderr);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        strcpy(output, "Use the Exit button or close the window to exit.\r\n");
        DeleteFileA(tempFilename);
        return;
    }
    else {
        printf("Unknown command: %s\r\n", token);
        printf("Type 'help' for available commands.\r\n");
    }
    
    /* Restore stdout and read captured output */
    fflush(stdout);
    fclose(stdout);
    fclose(stderr);
    
    /* Read temp file contents */
    FILE *temp = fopen(tempFilename, "r");
    if (temp) {
        size_t n = fread(output, 1, 4095, temp);
        output[n] = '\0';
        fclose(temp);
    } else {
        output[0] = '\0';
    }
    
    /* Restore stdout/stderr */
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    
    /* Clean up temp file */
    DeleteFileA(tempFilename);
}
