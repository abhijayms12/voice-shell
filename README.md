# Voice Enabled Command Line Interpreter - GUI

## Overview
Windows-native GUI application using Win32 API in C for Operating Systems Lab project.

## Features
- Terminal-like interface (800x600 window)
- Black background with white monospace text (Consolas font)
- Scrollable, read-only output area
- Single-line command input box
- "Run" button for command execution
- "Mic" button (placeholder for voice input)

## Compilation

### Using GCC (MinGW)
```bash
gcc gui.c -o voice_shell.exe -mwindows -lgdi32
```

### Using Makefile
```bash
mingw32-make
```

## Running
```bash
voice_shell.exe
```

## Backend Integration

The GUI expects a function to be implemented by the backend team:

```c
void execute_command(const char *input, char *output);
```

**Parameters:**
- `input`: Command string entered by user
- `output`: Buffer where command output should be written (max 4096 bytes)

**Integration Steps:**
1. Implement `execute_command()` in a separate C file (e.g., `backend.c`)
2. Compile together: `gcc gui.c backend.c -o voice_shell.exe -mwindows -lgdi32`

## Voice Recognition Integration

The "Mic" button calls `HandleMicButton()` function, which currently shows a placeholder message. This can be integrated with Python Whisper backend via:
- Named pipes
- Shared memory
- Socket communication
- System calls

## Code Structure

- `WinMain()`: Application entry point
- `WindowProc()`: Message handler
- `CreateControls()`: Creates all GUI elements
- `HandleRunCommand()`: Processes Run button clicks
- `HandleMicButton()`: Placeholder for voice input
- `AppendToOutput()`: Adds text to output area
- `execute_command()`: Backend integration point (declared, not implemented)

## Requirements
- Windows OS
- MinGW GCC compiler
- Win32 API (included with Windows SDK)

## Notes
- Pure C implementation (no C++)
- No external GUI frameworks
- Memory-safe design
- Ready for backend integration
