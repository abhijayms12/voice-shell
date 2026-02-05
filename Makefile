# Makefile for Voice Enabled Command Line Interpreter
# For MinGW GCC on Windows

CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -mwindows -lgdi32
TARGET = voice_shell.exe

# Production build with full backend
SOURCES = gui.c backend.c

# Default target
all: $(TARGET)

# Build executable
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)
	
# Build with stub backend for testing
stub: gui.c backend_stub.c
	$(CC) $(CFLAGS) gui.c backend_stub.c -o voice_shell_stub.exe $(LDFLAGS)

# Clean build artifacts
clean:
	del /Q $(TARGET) 2>nul || true

# Run the application
run: $(TARGET)
	.\$(TARGET)

.PHONY: all clean run
