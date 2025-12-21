# Makefile for Voice Enabled Command Line Interpreter
# For MinGW GCC on Windows

CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -mwindows -lgdi32
TARGET = voice_shell.exe
SOURCES = gui.c commands.c

# Default target
all: $(TARGET)

# Build executable
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

# Clean build artifacts
clean:
	del /Q $(TARGET) 2>nul || true

# Run the application
run: $(TARGET)
	.\$(TARGET)

.PHONY: all clean run
