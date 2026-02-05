# How to Run the Voice Shell Application

## Quick Start (Easiest Method)

**Simply double-click** `test.bat` - it will compile and launch automatically!

---

## Manual Compilation & Running

### Method 1: Using the Makefile

```bash
mingw32-make
voice_shell.exe
```

### Method 2: Direct GCC Compilation

```bash
gcc gui.c backend.c -o voice_shell.exe -mwindows -lgdi32
voice_shell.exe
```

---

## Prerequisites

1. **Windows Operating System**
2. **MinGW GCC compiler** installed and in PATH
   - Download from: https://www.mingw-w64.org/
   - Or use MSYS2: https://www.msys2.org/

3. **Python 3.x** (for voice recognition feature - optional)
   - Install from: https://www.python.org/

4. **Python dependencies** (for voice - optional):
   ```bash
   pip install -r requirements.txt
   ```

---

## Verifying MinGW Installation

Open PowerShell or Command Prompt and run:

```bash
gcc --version
```

If you see version information, you're ready to go!

If not, install MinGW and add it to your PATH environment variable.

---

## Using the Application

### Text Command Input

1. Type commands directly in the terminal area
2. Press **Enter** to execute
3. View output in the same terminal window

### Voice Command Input

1. Click the **"Voice Input"** button
2. Speak your command clearly
3. The transcribed command will be shown
4. Review and execute

---

## Available Commands

### Built-in Commands (Fast, No External Process)

| Command | Description | Example |
|---------|-------------|---------|
| `ls [path]` | List directory contents | `ls` or `ls C:\` |
| `dir [path]` | Alias for ls | `dir Desktop` |
| `pwd` | Print working directory | `pwd` |
| `cd [path]` | Change directory | `cd Documents` |
| `cat <file>` | Display file contents | `cat test.txt` |
| `type <file>` | Alias for cat | `type README.md` |
| `mkdir <dir>` | Create directory | `mkdir newfolder` |
| `rmdir <dir>` | Remove empty directory | `rmdir oldfolder` |
| `touch <file>` | Create/update file | `touch newfile.txt` |
| `rm <file>` | Remove file | `rm oldfile.txt` |
| `cp <src> <dest>` | Copy file | `cp file.txt backup.txt` |
| `mv <src> <dest>` | Move/rename file | `mv old.txt new.txt` |
| `clear` / `cls` | Clear screen | `clear` |
| `help` | Show help message | `help` |
| `exit` | Close application | `exit` |

### External Commands (Any Windows Command)

The shell can execute **any Windows command** via `cmd.exe`:

```bash
ipconfig /all
ping google.com
python script.py
dir /b
echo Hello World
systeminfo
tasklist
netstat -an
python --version
node --version
git status
```

---

## Command History

- **Up Arrow** - Previous command
- **Down Arrow** - Next command
- History persists for the session (up to 50 commands)

---

## Architecture Overview

```
┌─────────────────────────────────────┐
│         GUI Layer (gui.c)           │
│   - Win32 API terminal interface    │
│   - Input handling & display        │
└───────────┬─────────────────────────┘
            │
            ▼
┌─────────────────────────────────────┐
│     Backend Layer (backend.c)       │
│   - Built-in command execution      │
│   - External command via cmd.exe    │
│   - Output capture & buffering      │
└─────────────────────────────────────┘
```

---

## File Structure

```
voice_shell/
├── gui.c                  ← GUI implementation (Win32 API)
├── backend.c              ← Full backend (built-in + external commands)
├── backend_stub.c         ← Simple test backend (legacy)
├── commands.c             ← Alternative backend (legacy)
├── commands.h             ← Header for commands.c
├── voice_shell.exe        ← Compiled executable
├── whisper_once.py        ← Voice recognition script
├── requirements.txt       ← Python dependencies
├── test.bat               ← Quick compile & run script
├── Makefile               ← Build configuration
├── README.md              ← Project overview
├── RUN_INSTRUCTIONS.md    ← This file
├── QUICKREF.md            ← Quick reference card
├── ARCHITECTURE.md        ← System architecture
└── INTEGRATION_GUIDE.md   ← Integration instructions
```

---

## Troubleshooting

### "gcc is not recognized as an internal or external command"

**Solution:** Install MinGW and add it to your PATH

1. Download MinGW from https://www.mingw-w64.org/
2. Install to `C:\mingw64`
3. Add `C:\mingw64\bin` to your PATH environment variable
4. Restart terminal/PowerShell

### Compilation Errors

**Solution:** Ensure all source files are present

```bash
# Check files exist
ls gui.c backend.c

# Clean build
mingw32-make clean
mingw32-make
```

### Voice Input Not Working

**Solution:** Install Python dependencies

```bash
pip install openai-whisper sounddevice numpy
```

Check that `whisper_once.py` exists in the same directory.

### Application Won't Launch

**Solution:** Run from terminal to see error messages

```bash
# Instead of double-clicking, run from PowerShell:
.\voice_shell.exe
```

---

## Building Different Versions

### Production Build (Full Backend)
```bash
gcc gui.c backend.c -o voice_shell.exe -mwindows -lgdi32
```

### Test Build (Simple Stub)
```bash
gcc gui.c backend_stub.c -o voice_shell_stub.exe -mwindows -lgdi32
```

### Alternative Build (Legacy Commands)
```bash
gcc gui.c commands.c -o voice_shell_alt.exe -mwindows -lgdi32
```

---

## Development Notes

- **GUI Layer:** Pure C with Win32 API (no external frameworks)
- **Backend:** Native Windows APIs for process management
- **Voice:** Python + Whisper for speech recognition
- **Communication:** File-based IPC between C and Python

---

## For Developers

### Backend Interface

The backend must implement these two functions:

```c
void get_current_directory(char *buffer, int size);
void execute_command(const char *input, char *output);
```

### Adding New Built-in Commands

Edit `backend.c` and add your command handler:

1. Create handler function: `static void builtin_mycommand(const char *arg, char *output)`
2. Add condition in `execute_command()` function
3. Recompile

### Testing

```bash
# Compile
mingw32-make

# Run
voice_shell.exe

# Test commands
help
ls
pwd
cd ..
ipconfig
```

---

## Performance Characteristics

- **Built-in commands:** < 1ms execution time
- **External commands:** 50-200ms overhead (cmd.exe startup)
- **Voice recognition:** 2-5 seconds (depends on audio length)
- **Memory usage:** ~5MB (GUI) + variable (external processes)

---

## License & Credits

Operating Systems Lab Project  
Voice Enabled Command Line Interpreter  
Built with Win32 API and Python Whisper

---

## Need Help?

Run `help` command in the shell for available commands.

Check the following documentation:
- [README.md](README.md) - Project overview
- [ARCHITECTURE.md](ARCHITECTURE.md) - System design
- [INTEGRATION_GUIDE.md](INTEGRATION_GUIDE.md) - Integration details
- [QUICKREF.md](QUICKREF.md) - Quick reference card
