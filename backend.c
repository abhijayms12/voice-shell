/*
 * Voice Shell Backend - Production Implementation
 * 
 * Provides command execution for GUI layer with:
 * - Built-in shell commands (cd, ls, pwd, etc.)
 * - External program execution (via cmd.exe)
 * - Output capture and buffering
 * 
 * Compile with GUI:
 * gcc gui.c backend.c -o voice_shell.exe -mwindows -lgdi32
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <direct.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

/* Maximum buffer sizes */
#define MAX_OUTPUT_BUFFER 4096
#define MAX_PATH_LEN 1024
#define MAX_COMMAND_LEN 1024

/* ============================================================================
 * BUILT-IN COMMAND IMPLEMENTATIONS
 * ============================================================================ */

/* 
 * List directory contents with detailed formatting 
 */
static void builtin_ls(const char *path, char *output) {
    WIN32_FIND_DATAA de;
    HANDLE d;
    struct stat buf;
    char timebuf[32];
    char searchPath[MAX_PATH];
    char oldcwd[MAX_PATH];
    char result[MAX_OUTPUT_BUFFER] = "";
    
    if (path == NULL || strlen(path) == 0)
        path = ".";
    
    snprintf(searchPath, MAX_PATH, "%s\\*", path);
    
    d = FindFirstFileA(searchPath, &de);
    if (d == INVALID_HANDLE_VALUE) {
        sprintf(output, "Error: Cannot access '%s'\r\n", path);
        return;
    }
    
    /* Save and change directory for stat to work properly */
    _getcwd(oldcwd, sizeof(oldcwd));
    _chdir(path);
    
    strcat(result, "Name                           Type       Size        Modified\r\n");
    strcat(result, "------------------------------------------------------------------------\r\n");
    
    do {
        /* Skip . and .. */
        if (strcmp(de.cFileName, ".") == 0 || strcmp(de.cFileName, "..") == 0)
            continue;
        
        if (stat(de.cFileName, &buf) != 0)
            continue;
        
        /* Format modification time */
        struct tm *t = localtime(&buf.st_mtime);
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", t);
        
        /* Format output line */
        char line[256];
        if (S_ISDIR(buf.st_mode)) {
            sprintf(line, "%-30s <DIR>      %-11s %s\r\n", de.cFileName, "", timebuf);
        } else {
            sprintf(line, "%-30s FILE       %-11ld %s\r\n", de.cFileName, (long)buf.st_size, timebuf);
        }
        
        /* Check buffer space before appending */
        if (strlen(result) + strlen(line) < MAX_OUTPUT_BUFFER - 1) {
            strcat(result, line);
        }
        
    } while (FindNextFileA(d, &de));
    
    FindClose(d);
    _chdir(oldcwd);
    
    strcpy(output, result);
}

/* 
 * Print working directory 
 */
static void builtin_pwd(char *output) {
    char cwd[MAX_PATH];
    if (_getcwd(cwd, sizeof(cwd)) != NULL)
        sprintf(output, "%s\r\n", cwd);
    else
        strcpy(output, "Error: Cannot get current directory\r\n");
}

/* 
 * Change directory 
 */
static void builtin_cd(const char *path, char *output) {
    if (!path || strlen(path) == 0) {
        /* No argument - go to home directory */
        const char *home = getenv("USERPROFILE");
        if (!home) home = "C:\\";
        if (_chdir(home) != 0) {
            strcpy(output, "Error: Cannot change to home directory\r\n");
        } else {
            sprintf(output, "Changed to: %s\r\n", home);
        }
        return;
    }
    
    /* Change to specified directory */
    if (_chdir(path) != 0) {
        sprintf(output, "Error: Cannot access '%s'\r\n", path);
    } else {
        char cwd[MAX_PATH];
        _getcwd(cwd, sizeof(cwd));
        sprintf(output, "Changed to: %s\r\n", cwd);
    }
}

/* 
 * Display file contents 
 */
static void builtin_cat(const char *filename, char *output) {
    if (!filename || strlen(filename) == 0) {
        strcpy(output, "Error: cat requires a filename\r\n");
        return;
    }
    
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        sprintf(output, "Error: Cannot open file '%s'\r\n", filename);
        return;
    }
    
    /* Read file content */
    size_t n = fread(output, 1, MAX_OUTPUT_BUFFER - 100, fp);
    output[n] = '\0';
    
    /* Add newline if file doesn't end with one */
    if (n > 0 && output[n-1] != '\n') {
        strcat(output, "\r\n");
    }
    
    fclose(fp);
}

/* 
 * Create new directory 
 */
static void builtin_mkdir(const char *dirname, char *output) {
    if (!dirname || strlen(dirname) == 0) {
        strcpy(output, "Error: mkdir requires a directory name\r\n");
        return;
    }
    
    if (_mkdir(dirname) != 0) {
        sprintf(output, "Error: Cannot create directory '%s'\r\n", dirname);
    } else {
        sprintf(output, "Directory created: %s\r\n", dirname);
    }
}

/* 
 * Remove empty directory 
 */
static void builtin_rmdir(const char *dirname, char *output) {
    if (!dirname || strlen(dirname) == 0) {
        strcpy(output, "Error: rmdir requires a directory name\r\n");
        return;
    }
    
    if (_rmdir(dirname) != 0) {
        sprintf(output, "Error: Cannot remove directory '%s'\r\n", dirname);
    } else {
        sprintf(output, "Directory removed: %s\r\n", dirname);
    }
}

/* 
 * Create or update file timestamp 
 */
static void builtin_touch(const char *filename, char *output) {
    if (!filename || strlen(filename) == 0) {
        strcpy(output, "Error: touch requires a filename\r\n");
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
        sprintf(output, "Error: Cannot create or update file '%s'\r\n", filename);
        return;
    }
    
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    SetFileTime(hFile, NULL, NULL, &ft);
    CloseHandle(hFile);
    
    sprintf(output, "File created/updated: %s\r\n", filename);
}

/* 
 * Remove file 
 */
static void builtin_rm(const char *filename, char *output) {
    if (!filename || strlen(filename) == 0) {
        strcpy(output, "Error: rm requires a filename\r\n");
        return;
    }
    
    if (remove(filename) != 0) {
        sprintf(output, "Error: Cannot remove file '%s'\r\n", filename);
    } else {
        sprintf(output, "File removed: %s\r\n", filename);
    }
}

/* 
 * Helper function to extract basename from path 
 */
static const char *get_basename(const char *path) {
    const char *p = strrchr(path, '\\');
    if (!p) p = strrchr(path, '/');
    return p ? p + 1 : path;
}

/* 
 * Copy file 
 */
static void builtin_cp(const char *src, const char *dest, char *output) {
    char target[MAX_PATH];
    DWORD destAttr;
    
    if (!src || !dest) {
        strcpy(output, "Error: cp requires source and destination\r\n");
        return;
    }
    
    /* Check if source exists */
    if (GetFileAttributesA(src) == INVALID_FILE_ATTRIBUTES) {
        sprintf(output, "Error: source file not found '%s'\r\n", src);
        return;
    }
    
    /* If destination is a directory, append source filename */
    destAttr = GetFileAttributesA(dest);
    if (destAttr != INVALID_FILE_ATTRIBUTES && (destAttr & FILE_ATTRIBUTE_DIRECTORY)) {
        snprintf(target, sizeof(target), "%s\\%s", dest, get_basename(src));
    } else {
        strncpy(target, dest, sizeof(target) - 1);
        target[sizeof(target) - 1] = '\0';
    }
    
    /* Perform copy (overwrite if exists) */
    if (CopyFileA(src, target, FALSE)) {
        sprintf(output, "Copied '%s' -> '%s'\r\n", src, target);
    } else {
        sprintf(output, "Error: Cannot copy '%s' to '%s'\r\n", src, target);
    }
}

/* 
 * Move/rename file 
 */
static void builtin_mv(const char *src, const char *dest, char *output) {
    char target[MAX_PATH];
    DWORD destAttr;
    
    if (!src || !dest) {
        strcpy(output, "Error: mv requires source and destination\r\n");
        return;
    }
    
    /* Check if source exists */
    if (GetFileAttributesA(src) == INVALID_FILE_ATTRIBUTES) {
        sprintf(output, "Error: source file not found '%s'\r\n", src);
        return;
    }
    
    /* If destination is a directory, append source filename */
    destAttr = GetFileAttributesA(dest);
    if (destAttr != INVALID_FILE_ATTRIBUTES && (destAttr & FILE_ATTRIBUTE_DIRECTORY)) {
        snprintf(target, sizeof(target), "%s\\%s", dest, get_basename(src));
    } else {
        strncpy(target, dest, sizeof(target) - 1);
        target[sizeof(target) - 1] = '\0';
    }
    
    /* Try native move first */
    if (MoveFileExA(src, target, MOVEFILE_REPLACE_EXISTING)) {
        sprintf(output, "Moved '%s' -> '%s'\r\n", src, target);
        return;
    }
    
    /* If move failed, try copy + delete (for cross-volume moves) */
    if (CopyFileA(src, target, FALSE)) {
        if (DeleteFileA(src)) {
            sprintf(output, "Moved '%s' -> '%s'\r\n", src, target);
        } else {
            sprintf(output, "Warning: copied to '%s' but failed to remove '%s'\r\n", target, src);
        }
    } else {
        sprintf(output, "Error: Cannot move '%s' to '%s'\r\n", src, target);
    }
}

/* 
 * Clear screen (signal to GUI) 
 */
static void builtin_clear(char *output) {
    strcpy(output, "::CLEAR_SCREEN::\r\n");
}

/* 
 * Display help information 
 */
static void builtin_help(char *output) {
    strcpy(output, 
        "Voice Enabled Command Line Interpreter - Available Commands:\r\n\r\n"
        "Built-in Commands:\r\n"
        "  ls [path]           - List files and directories\r\n"
        "  dir [path]          - Alias for ls\r\n"
        "  pwd                 - Print current working directory\r\n"
        "  cd [path]           - Change directory\r\n"
        "  cat <file>          - Display file contents\r\n"
        "  type <file>         - Alias for cat\r\n"
        "  mkdir <dir>         - Create new directory\r\n"
        "  rmdir <dir>         - Remove empty directory\r\n"
        "  touch <file>        - Create or update file\r\n"
        "  rm <file>           - Remove file\r\n"
        "  cp <src> <dest>     - Copy file\r\n"
        "  mv <src> <dest>     - Move/rename file\r\n"
        "  clear / cls         - Clear the screen\r\n"
        "  help                - Show this help message\r\n"
        "  exit                - Close the application\r\n\r\n"
        "External Commands:\r\n"
        "  Any Windows command (ipconfig, ping, python, etc.)\r\n"
        "  Example: ipconfig /all\r\n"
        "  Example: python script.py\r\n"
        "  Example: dir /b\r\n"
    );
}

/* ============================================================================
 * EXTERNAL COMMAND EXECUTION
 * ============================================================================ */

/*
 * Execute external command via cmd.exe and capture output
 */
static void execute_external_command(const char *command, char *output) {
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    char cmdLine[MAX_COMMAND_LEN];
    DWORD bytesRead;
    char buffer[4096];
    
    /* Create pipe for output capture */
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        strcpy(output, "Error: Cannot create pipe for command output\r\n");
        return;
    }
    
    /* Ensure read handle is not inherited */
    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);
    
    /* Setup process startup info */
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    si.wShowWindow = SW_HIDE;
    
    /* Build command line: cmd.exe /c "command" */
    snprintf(cmdLine, sizeof(cmdLine), "cmd.exe /c \"%s\"", command);
    
    /* Create process */
    if (!CreateProcessA(NULL, cmdLine, NULL, NULL, TRUE, 
                       CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        sprintf(output, "Error: Cannot execute command '%s'\r\n", command);
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return;
    }
    
    /* Close write end in parent process */
    CloseHandle(hWritePipe);
    
    /* Read output from pipe */
    output[0] = '\0';
    size_t totalRead = 0;
    
    while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        
        /* Check buffer space */
        if (totalRead + bytesRead < MAX_OUTPUT_BUFFER - 1) {
            strcat(output, buffer);
            totalRead += bytesRead;
        } else {
            /* Truncate if too much output */
            strncat(output, buffer, MAX_OUTPUT_BUFFER - totalRead - 50);
            strcat(output, "\r\n... (output truncated)\r\n");
            break;
        }
    }
    
    /* Wait for process to complete */
    WaitForSingleObject(pi.hProcess, INFINITE);
    
    /* Get exit code */
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    
    /* Cleanup */
    CloseHandle(hReadPipe);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    /* If no output and non-zero exit code, indicate error */
    if (strlen(output) == 0 && exitCode != 0) {
        sprintf(output, "Command failed with exit code %lu\r\n", exitCode);
    }
}

/* ============================================================================
 * COMMAND PARSING AND DISPATCH
 * ============================================================================ */

/*
 * Trim leading and trailing whitespace from string
 */
static void trim(char *str) {
    char *start = str;
    char *end;
    
    /* Trim leading space */
    while (isspace((unsigned char)*start)) start++;
    
    /* All spaces? */
    if (*start == 0) {
        str[0] = '\0';
        return;
    }
    
    /* Trim trailing space */
    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    
    /* Write new null terminator */
    end[1] = '\0';
    
    /* Move trimmed string to start of buffer */
    memmove(str, start, strlen(start) + 1);
}

/*
 * Parse command line into command and arguments
 * Returns: number of arguments parsed
 */
static int parse_command(const char *input, char *cmd, char *arg1, char *arg2) {
    char buffer[MAX_COMMAND_LEN];
    char *token;
    int argc = 0;
    
    /* Initialize outputs */
    cmd[0] = '\0';
    arg1[0] = '\0';
    arg2[0] = '\0';
    
    /* Copy input to buffer */
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    /* Parse tokens */
    token = strtok(buffer, " \t");
    if (token) {
        strcpy(cmd, token);
        argc++;
        
        token = strtok(NULL, " \t");
        if (token) {
            strcpy(arg1, token);
            argc++;
            
            /* For commands like cp/mv, get remaining text as second arg */
            token = strtok(NULL, "");
            if (token) {
                /* Trim leading spaces */
                while (*token == ' ' || *token == '\t') token++;
                strcpy(arg2, token);
                argc++;
            }
        }
    }
    
    return argc;
}

/* ============================================================================
 * GUI INTEGRATION INTERFACE
 * ============================================================================ */

/*
 * Get current working directory for prompt display
 * Called by GUI to update prompt
 */
void get_current_directory(char *buffer, int size) {
    if (_getcwd(buffer, size) == NULL) {
        strncpy(buffer, "C:\\", size - 1);
        buffer[size - 1] = '\0';
    }
}

/*
 * Main command execution entry point
 * Called by GUI when user submits a command
 * 
 * Parameters:
 *   input  - User's command string (null-terminated)
 *   output - Buffer for command output (4096 bytes)
 */
void execute_command(const char *input, char *output) {
    char cmd[256];
    char arg1[512];
    char arg2[512];
    char inputCopy[MAX_COMMAND_LEN];
    
    if (!input || !output) {
        return;
    }
    
    /* Initialize output */
    output[0] = '\0';
    
    /* Copy and trim input */
    strncpy(inputCopy, input, sizeof(inputCopy) - 1);
    inputCopy[sizeof(inputCopy) - 1] = '\0';
    trim(inputCopy);
    
    /* Empty command */
    if (strlen(inputCopy) == 0) {
        return;
    }
    
    /* Parse command and arguments */
    parse_command(inputCopy, cmd, arg1, arg2);
    
    /* Convert command to lowercase for comparison */
    char cmdLower[256];
    strncpy(cmdLower, cmd, sizeof(cmdLower) - 1);
    cmdLower[sizeof(cmdLower) - 1] = '\0';
    _strlwr(cmdLower);
    
    /* ========== BUILT-IN COMMANDS ========== */
    
    if (strcmp(cmdLower, "ls") == 0 || strcmp(cmdLower, "dir") == 0) {
        builtin_ls(strlen(arg1) > 0 ? arg1 : ".", output);
    }
    else if (strcmp(cmdLower, "pwd") == 0) {
        builtin_pwd(output);
    }
    else if (strcmp(cmdLower, "cd") == 0) {
        builtin_cd(strlen(arg1) > 0 ? arg1 : NULL, output);
    }
    else if (strcmp(cmdLower, "cat") == 0 || strcmp(cmdLower, "type") == 0) {
        builtin_cat(arg1, output);
    }
    else if (strcmp(cmdLower, "mkdir") == 0) {
        builtin_mkdir(arg1, output);
    }
    else if (strcmp(cmdLower, "rmdir") == 0) {
        builtin_rmdir(arg1, output);
    }
    else if (strcmp(cmdLower, "touch") == 0) {
        builtin_touch(arg1, output);
    }
    else if (strcmp(cmdLower, "rm") == 0) {
        builtin_rm(arg1, output);
    }
    else if (strcmp(cmdLower, "cp") == 0) {
        builtin_cp(arg1, arg2, output);
    }
    else if (strcmp(cmdLower, "mv") == 0) {
        builtin_mv(arg1, arg2, output);
    }
    else if (strcmp(cmdLower, "clear") == 0 || strcmp(cmdLower, "cls") == 0) {
        builtin_clear(output);
    }
    else if (strcmp(cmdLower, "help") == 0) {
        builtin_help(output);
    }
    else if (strcmp(cmdLower, "exit") == 0) {
        strcpy(output, "Use the window close button (X) to exit.\r\n");
    }
    /* ========== EXTERNAL COMMANDS ========== */
    else {
        /* Execute as external Windows command */
        execute_external_command(inputCopy, output);
    }
}
