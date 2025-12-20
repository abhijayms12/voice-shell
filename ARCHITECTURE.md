# System Architecture

## Component Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Voice Shell System                        │
│                  Operating Systems Lab Project               │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                         GUI Layer (C)                        │
│                    [gui.c - Your Responsibility]             │
│                                                              │
│  ┌────────────────────────────────────────────────────┐    │
│  │  Main Window (800x600)                             │    │
│  │  ┌──────────────────────────────────────────────┐  │    │
│  │  │  Output Area                                 │  │    │
│  │  │  - Black background                          │  │    │
│  │  │  - White monospace text (Consolas)           │  │    │
│  │  │  - Read-only, scrollable                     │  │    │
│  │  │  - Shows: $ command + output                 │  │    │
│  │  └──────────────────────────────────────────────┘  │    │
│  │  ┌──────────────────────────────────────────────┐  │    │
│  │  │  Input Box: [_____________________]          │  │    │
│  │  └──────────────────────────────────────────────┘  │    │
│  │  [Run Button]  [Mic Button]                        │    │
│  └────────────────────────────────────────────────────┘    │
│                           │                                  │
│                           │ User Actions:                    │
│                           │ 1. Type command                  │
│                           │ 2. Click Run / Press Enter       │
│                           │ 3. Click Mic (voice input)       │
└───────────────────────────┼──────────────────────────────────┘
                            │
            ┌───────────────┴───────────────┐
            │                               │
            ▼                               ▼
┌────────────────────────┐      ┌─────────────────────────┐
│  Backend: Command      │      │  Backend: Voice Input   │
│  Execution (C)         │      │  (Python + Whisper)     │
│  [backend.c]           │      │  [whisper_module.py]    │
│                        │      │                         │
│  execute_command()     │      │  - Audio recording      │
│  ├─ Parse input        │      │  - Whisper transcribe   │
│  ├─ Execute logic      │      │  - Return text to GUI   │
│  └─ Return output      │      │                         │
│                        │      │  Communication:         │
│  Examples:             │      │  - Named pipe           │
│  - File operations     │      │  - System call + file   │
│  - Process mgmt        │      │  - Shared memory        │
│  - Custom commands     │      │                         │
└────────────────────────┘      └─────────────────────────┘
```

## Data Flow

### Command Execution Flow
```
1. User types command in Input Box
              ↓
2. User clicks [Run Button]
              ↓
3. GUI calls: execute_command(input, output)
              ↓
4. Backend processes command (backend.c)
              ↓
5. Backend writes result to output buffer
              ↓
6. GUI displays: "$ command\n" + output
              ↓
7. GUI clears input box, ready for next command
```

### Voice Input Flow (Planned)
```
1. User clicks [Mic Button]
              ↓
2. GUI calls Python script / opens pipe
              ↓
3. Python records audio
              ↓
4. Whisper transcribes audio → text
              ↓
5. Text sent back to GUI
              ↓
6. GUI populates Input Box with transcribed text
              ↓
7. User reviews and clicks [Run]
```

## File Organization

```
voice_shell/
│
├── gui.c                    ← GUI implementation (Win32 API)
│   ├── WinMain()           - Entry point
│   ├── WindowProc()        - Message handler
│   ├── CreateControls()    - Build UI elements
│   ├── HandleRunCommand()  - Process command execution
│   └── HandleMicButton()   - Voice input trigger
│
├── backend.c                ← Command execution (teammate)
│   └── execute_command()   - Command parsing & execution
│
├── backend_stub.c           ← Temporary test stub
│   └── execute_command()   - Mock implementation
│
├── whisper_module.py        ← Voice recognition (teammate)
│   ├── record_audio()
│   ├── transcribe()
│   └── return_text()
│
├── voice_shell.exe          ← Compiled executable
├── Makefile                 ← Build configuration
├── README.md                ← Project overview
├── INTEGRATION_GUIDE.md     ← Integration instructions
├── ARCHITECTURE.md          ← This file
└── test.bat                 ← Quick test script
```

## Integration Points

### Point 1: Command Execution
**Location**: gui.c → backend.c  
**Interface**: `void execute_command(const char *input, char *output)`  
**Status**: ✓ Declared in GUI, awaiting backend implementation

### Point 2: Voice Recognition
**Location**: gui.c → whisper_module.py  
**Interface**: TBD (named pipe, file, or shared memory)  
**Status**: ⚠ Placeholder in place

## Compilation Chain

```
┌──────────┐     ┌──────────────┐     ┌────────────────┐
│  gui.c   │────→│   gcc        │────→│ voice_shell.exe│
└──────────┘     │  (MinGW)     │     └────────────────┘
                 │              │
┌──────────┐     │   Flags:     │
│backend.c │────→│   -mwindows  │
└──────────┘     │   -lgdi32    │
                 └──────────────┘

Command: gcc gui.c backend.c -o voice_shell.exe -mwindows -lgdi32
```

## Memory Management

```
Stack Buffers:
├── input[MAX_INPUT_SIZE]          = 1024 bytes
├── output[MAX_COMMAND_OUTPUT]     = 4096 bytes
└── displayBuffer[MAX_OUTPUT_SIZE] = 8192 bytes

Heap Allocations:
├── HFONT (Consolas font)          - Deleted on WM_DESTROY
├── HBRUSH (black background)      - Deleted on WM_DESTROY
└── Window handles                 - Managed by Win32 API
```

## Thread Model

```
Main Thread (GUI Thread):
├── Message loop (GetMessage/DispatchMessage)
├── Window procedure (WindowProc)
├── Control creation
├── Command execution (synchronous)
└── Voice input trigger (synchronous)

Future Enhancement:
└── Worker thread for long-running commands (optional)
```

## Testing Strategy

### Phase 1: GUI Standalone
```
✓ Compile with backend_stub.c
✓ Test UI elements
✓ Verify layout and colors
✓ Test button responses
```

### Phase 2: Backend Integration
```
□ Replace backend_stub.c with backend.c
□ Test command parsing
□ Test command execution
□ Test output display
```

### Phase 3: Voice Integration
```
□ Implement voice input communication
□ Test audio recording
□ Test Whisper transcription
□ Test text insertion to input box
```

### Phase 4: Full System
```
□ Test end-to-end: voice → command → output
□ Test error handling
□ Test edge cases
□ Performance testing
```

## Design Decisions

### Why Win32 API?
- Native Windows performance
- No external dependencies
- Direct OS integration
- Required by project specification

### Why Single-Threaded?
- Simplicity for lab project
- Sufficient for command execution
- Easy to debug
- Can be extended to multi-threaded if needed

### Why Separate Backend?
- Modular design
- Parallel development
- Easy testing (use stub)
- Clear responsibilities

### Why File-Based Voice Communication?
- Simple and reliable
- No complex IPC setup
- Cross-language compatible
- Easy to debug

## Security Considerations

```
Input Validation:
├── Max input size: 1024 bytes
├── Buffer overflow protection: bounds checking
└── Command injection: backend responsibility

Memory Safety:
├── All buffers pre-allocated
├── No dynamic memory in critical path
└── Cleanup on window close

Process Isolation:
├── Voice module runs separately
├── Backend can be sandboxed
└── No elevated privileges required
```

## Performance Characteristics

```
Window Creation: < 100ms
Command Execution: Depends on backend
Output Display: < 10ms
Window Resize: < 50ms
Voice Input: Depends on Python/Whisper (2-5 seconds typical)
Memory Usage: < 5 MB (GUI only)
```

## Future Enhancements (Optional)

```
□ Command history (↑/↓ arrows)
□ Tab completion
□ Multi-line input
□ Syntax highlighting
□ Copy/paste support
□ Save output to file
□ Configurable colors
□ Custom fonts
□ Keyboard shortcuts
□ Real-time voice streaming
```

---

**Note**: This architecture is designed for an Operating Systems Lab project. Keep it simple, modular, and focused on the core requirements.
