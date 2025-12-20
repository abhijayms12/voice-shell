# Backend Integration Guide

## For Command Execution Backend (C)

### Function Signature
```c
void execute_command(const char *input, char *output);
```

### Parameters
- **input**: Null-terminated command string from user (max 1024 bytes)
- **output**: Pre-allocated buffer for command output (4096 bytes)

### Requirements
1. Read the command from `input`
2. Execute the command logic
3. Write the result to `output` buffer
4. Ensure null-termination
5. Do NOT exceed buffer size (4096 bytes)

### Example Implementation
```c
// backend.c
#include <string.h>
#include <stdio.h>

void execute_command(const char *input, char *output) {
    // Your command parsing logic here
    if (strcmp(input, "ls") == 0) {
        // List files
        strcpy(output, "file1.txt\nfile2.txt\nfile3.txt\n");
    }
    else if (strncmp(input, "echo ", 5) == 0) {
        // Echo command
        strcpy(output, input + 5);
        strcat(output, "\n");
    }
    else {
        sprintf(output, "Unknown command: %s\n", input);
    }
}
```

### Compilation
```bash
gcc gui.c backend.c -o voice_shell.exe -mwindows -lgdi32
```

### Important Notes
- The GUI handles all display formatting (e.g., "$ command")
- Just write the command output to the buffer
- Newlines (\n) are respected in the GUI
- The output area scrolls automatically
- No need to clear previous output - GUI manages this

---

## For Voice Recognition Backend (Python)

### Overview
The Mic button currently calls `HandleMicButton()` which is a placeholder. You can integrate Python Whisper in several ways:

### Option 1: Named Pipe Communication
**C Side (modify HandleMicButton):**
```c
void HandleMicButton(HWND hwnd) {
    // Signal Python process via named pipe
    HANDLE hPipe = CreateFile("\\\\.\\pipe\\whisper_pipe", 
                             GENERIC_WRITE, 0, NULL, 
                             OPEN_EXISTING, 0, NULL);
    if (hPipe != INVALID_HANDLE_VALUE) {
        char message[] = "START_RECORDING";
        DWORD written;
        WriteFile(hPipe, message, strlen(message), &written, NULL);
        CloseHandle(hPipe);
    }
}
```

**Python Side:**
```python
import win32pipe, win32file
import whisper

# Create named pipe server
pipe = win32pipe.CreateNamedPipe(
    r'\\.\pipe\whisper_pipe',
    win32pipe.PIPE_ACCESS_DUPLEX,
    win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_WAIT,
    1, 65536, 65536, 0, None)

# Wait for connection
win32pipe.ConnectNamedPipe(pipe, None)

# Receive command and process
data = win32file.ReadFile(pipe, 4096)
if data == "START_RECORDING":
    # Record audio and transcribe with Whisper
    # Write result back to GUI via pipe or file
```

### Option 2: System Call
**C Side:**
```c
void HandleMicButton(HWND hwnd) {
    system("python whisper_recorder.py");
    
    // Read result from temporary file
    FILE *f = fopen("voice_input.txt", "r");
    if (f) {
        char command[1024];
        fgets(command, 1024, f);
        fclose(f);
        
        // Put command in input box
        SetWindowText(hInputBox, command);
    }
}
```

**Python Side:**
```python
# whisper_recorder.py
import whisper
import sounddevice as sd
import numpy as np

# Record audio
audio = sd.rec(int(5 * 16000), samplerate=16000, channels=1)
sd.wait()

# Transcribe with Whisper
model = whisper.load_model("base")
result = model.transcribe(audio)

# Write to file
with open("voice_input.txt", "w") as f:
    f.write(result["text"])
```

### Option 3: Shared Memory
Use Windows shared memory objects for faster communication between C and Python.

### Recommended Approach
**For this lab project, Option 2 (System Call + File) is simplest:**
1. Mic button triggers Python script
2. Python records audio, uses Whisper to transcribe
3. Writes transcription to a file
4. C reads file and puts text in input box
5. User can review and press Run

### Testing Voice Integration
1. Replace `backend_stub.c` with your actual backend
2. Implement one of the voice options above
3. Compile and test: `gcc gui.c backend.c -o voice_shell.exe -mwindows -lgdi32`

---

## Testing Checklist

### GUI Functionality
- [x] Window opens at 800x600
- [x] Title shows "Voice Enabled Command Line Interpreter"
- [x] Output area has black background
- [x] Output area has white monospace text
- [x] Output area is read-only and scrollable
- [x] Input box accepts text
- [x] Run button executes commands
- [x] Mic button is present (placeholder)
- [x] Window can be resized
- [x] Controls resize with window

### Integration Points
- [ ] execute_command() implemented by backend team
- [ ] execute_command() receives input correctly
- [ ] execute_command() returns output correctly
- [ ] Voice input integration functional

### Memory & Performance
- [x] No memory leaks in GUI code
- [x] Buffer sizes are safe
- [x] Window closes cleanly

---

## Troubleshooting

### Compilation Errors
```bash
# Missing Win32 libraries
gcc gui.c backend.c -o voice_shell.exe -mwindows -lgdi32

# If Consolas font not found, it will fall back to default monospace
```

### Runtime Issues
- **Black screen**: Check GetClientRect call in CreateControls
- **Buttons not working**: Verify WM_COMMAND handling
- **No output display**: Check AppendToOutput function
- **Output not scrolling**: Verify ES_AUTOVSCROLL flag

### Backend Integration
- Ensure `execute_command()` is defined exactly as declared
- Check buffer sizes match (MAX_COMMAND_OUTPUT = 4096)
- Test with simple commands first
- Use printf debugging in backend if needed

---

## Project Structure
```
voice_shell/
├── gui.c                  # GUI implementation (this file)
├── backend.c              # Command execution (your teammate)
├── backend_stub.c         # Test stub (remove in final build)
├── voice_shell.exe        # Compiled executable
├── Makefile              # Build configuration
├── README.md             # Project overview
└── INTEGRATION_GUIDE.md  # This file
```

## Final Integration Steps
1. Replace `backend_stub.c` with actual `backend.c`
2. Implement voice recognition (optional for now)
3. Compile: `gcc gui.c backend.c -o voice_shell.exe -mwindows -lgdi32`
4. Test all functionality
5. Submit for lab evaluation

---

## Contact Points
- **GUI Issues**: Contact GUI developer (you)
- **Command Execution**: Contact backend C developer
- **Voice Recognition**: Contact Python/Whisper developer
