# Quick Reference Card

## Compilation Commands

### Basic (GUI + Backend Stub)
```bash
gcc gui.c backend_stub.c -o voice_shell.exe -mwindows -lgdi32
```

### Production (GUI + Real Backend)
```bash
gcc gui.c backend.c -o voice_shell.exe -mwindows -lgdi32
```

### Using Makefile
```bash
mingw32-make
```

## Running the Application
```bash
voice_shell.exe
```

Or double-click test.bat for quick testing.

## Backend Function Signature

```c
void execute_command(const char *input, char *output);
```

**You MUST implement this function in backend.c**

Example:
```c
void execute_command(const char *input, char *output) {
    if (strcmp(input, "hello") == 0) {
        strcpy(output, "Hello, World!\n");
    } else {
        sprintf(output, "Unknown command: %s\n", input);
    }
}
```

## File Structure

```
voice_shell/
├── gui.c              ← GUI (Win32 API) - COMPLETED ✓
├── backend_stub.c     ← Test stub (remove later)
├── backend.c          ← Real backend (YOUR TASK)
├── voice_shell.exe    ← Compiled program
├── Makefile           ← Build config
├── README.md          ← Overview
├── INTEGRATION_GUIDE.md  ← Detailed integration steps
├── ARCHITECTURE.md    ← System design
└── test.bat           ← Quick test script
```

## GUI Features Checklist

✓ 800x600 window  
✓ Title: "Voice Enabled Command Line Interpreter"  
✓ Black background for output  
✓ White Consolas monospace font  
✓ Scrollable, read-only output area  
✓ Single-line input box  
✓ "Run" button (executes commands)  
✓ "Mic" button (placeholder)  
✓ Displays "$ command" before output  
✓ Auto-scrolls output  
✓ Clears input after execution  
✓ No memory leaks  
✓ Resizable window  

## Testing Commands (with backend_stub.c)

| Command | Output |
|---------|--------|
| `help` | Shows available commands |
| `echo Hello` | Displays "Hello" |
| `date` | Shows current date |
| `anything_else` | "Command not recognized" message |

## Integration Steps

### Step 1: Test GUI Standalone
```bash
gcc gui.c backend_stub.c -o voice_shell.exe -mwindows -lgdi32
voice_shell.exe
```
Try commands: help, echo test, date

### Step 2: Replace Backend
1. Create backend.c with execute_command() implementation
2. Remove or replace backend_stub.c
3. Compile: `gcc gui.c backend.c -o voice_shell.exe -mwindows -lgdi32`
4. Test your commands

### Step 3: Add Voice Input (Optional)
1. Implement voice → text conversion in Python
2. Modify HandleMicButton() in gui.c to call Python
3. Use named pipe, file, or system call for communication
4. Test end-to-end

## Common Issues & Solutions

### Issue: "undefined reference to execute_command"
**Solution**: You forgot to include backend.c or backend_stub.c in compilation

### Issue: Commands don't work
**Solution**: Implement execute_command() in backend.c

### Issue: Window is blank
**Solution**: Check that CreateControls() is being called

### Issue: Output area is white, not black
**Solution**: Check WM_CTLCOLOREDIT message handler

### Issue: Can't compile - gcc not found
**Solution**: Install MinGW and add to PATH

## Key Constants

```c
WINDOW_WIDTH       = 800
WINDOW_HEIGHT      = 600
MAX_INPUT_SIZE     = 1024   // Input buffer size
MAX_COMMAND_OUTPUT = 4096   // Output buffer size
MAX_OUTPUT_SIZE    = 8192   // Display buffer size
```

## Important Functions

| Function | Purpose |
|----------|---------|
| WinMain() | Application entry point |
| WindowProc() | Handles Windows messages |
| CreateControls() | Creates UI elements |
| HandleRunCommand() | Processes command execution |
| HandleMicButton() | Placeholder for voice input |
| AppendToOutput() | Adds text to output area |
| execute_command() | **YOUR BACKEND FUNCTION** |

## Documentation Files

- **README.md** - Project overview and basic usage
- **INTEGRATION_GUIDE.md** - Detailed integration instructions  
- **ARCHITECTURE.md** - System design and data flow
- **QUICKREF.md** - This file (quick reference)

## Contact & Collaboration

- GUI issues → This developer (GUI complete)
- Command execution → Backend C developer (implement execute_command)
- Voice recognition → Python developer (integrate Whisper)

## Next Steps

1. ✓ GUI is complete and tested
2. → Implement execute_command() in backend.c
3. → Test command execution
4. → (Optional) Add voice input integration
5. → Final testing and submission

---

**Status**: GUI layer is READY FOR INTEGRATION ✓  
**Next**: Implement backend command execution logic

Happy coding! 🚀
