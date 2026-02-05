@echo off
REM Quick Test Script for Voice Shell GUI
REM This script compiles and runs the GUI with the production backend

echo ============================================
echo Voice Enabled Command Line Interpreter
echo Compilation and Run Script
echo ============================================
echo.

echo [1/2] Compiling GUI with production backend...
gcc gui.c backend.c -o voice_shell.exe -mwindows -lgdi32

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Compilation failed!
    echo Please check that MinGW GCC is installed and in PATH.
    pause
    exit /b 1
)

echo [OK] Compilation successful!
echo.

echo [2/2] Launching Voice Shell GUI...
echo.
echo Available Commands:
echo   Built-in: help, ls, pwd, cd, cat, mkdir, rm, cp, mv, clear
echo   External: Any Windows command (ipconfig, python, dir, etc.)
echo   Voice: Click "Voice Input" button to use speech recognition
echo.
echo Press Ctrl+C to stop or close the GUI window.
echo ============================================

start voice_shell.exe

echo.
echo GUI launched! Check the application window.
echo.
pause
