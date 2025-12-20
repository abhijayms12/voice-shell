/*
 * Backend Stub for Testing
 * This file provides temporary implementations for testing the GUI.
 * 
 * To compile with GUI:
 * gcc gui.c backend_stub.c -o voice_shell.exe -mwindows -lgdi32
 * 
 * This file should be REPLACED by the actual backend implementation.
 */

#include <string.h>
#include <stdio.h>
#include <windows.h>

/*
 * Stub implementation of current directory retrieval
 * Replace this with actual backend logic
 */
void get_current_directory(char *buffer, int size)
{
    /* Get actual current directory on Windows */
    if (GetCurrentDirectory(size, buffer) == 0) {
        strcpy(buffer, "C:\\");
    }
}

/*
 * Stub implementation of command execution
 * Replace this with actual backend logic
 */
void execute_command(const char *input, char *output)
{
    /* Simple test responses */
    if (strcmp(input, "help") == 0) {
        strcpy(output, "Available commands:\n  help - Show this message\n  echo <text> - Echo text\n  date - Show date\n");
    }
    else if (strncmp(input, "echo ", 5) == 0) {
        strcpy(output, input + 5);
        strcat(output, "\n");
    }
    else if (strcmp(input, "date") == 0) {
        strcpy(output, "December 20, 2025\n");
    }
    else if (strcmp(input, "clear") == 0) {
        strcpy(output, "");  /* Clear command handled by GUI if needed */
    }
    else {
        sprintf(output, "Command not recognized: %s\nType 'help' for available commands.\n", input);
    }
}
