/*
 * Voice Enabled Command Line Interpreter - GUI Layer
 * OS Lab Project - Win32 API Implementation
 * 
 * Compiler: gcc (MinGW)
 * Usage: gcc gui.c -o voice_shell.exe -mwindows -lgdi32
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>

/* Declare DPI awareness function (for older MinGW) */
typedef BOOL (WINAPI *SetProcessDPIAwareFunc)(void);

/* Dark mode constants for title bar */
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

/* Window dimensions */
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

/* Control IDs */
#define ID_OUTPUT_AREA  1001
#define ID_INPUT_BOX    1002
#define ID_RUN_BUTTON   1003
#define ID_MIC_BUTTON   1004
#define ID_PROMPT_TEXT  1005

/* Maximum buffer sizes */
#define MAX_OUTPUT_SIZE 8192
#define MAX_INPUT_SIZE  1024
#define MAX_COMMAND_OUTPUT 4096
#define MAX_PROMPT_SIZE 512

/* Global window handles */
HWND hOutputArea = NULL;
HWND hMicButton = NULL;

/* Font handle for monospace display */
HFONT hFont = NULL;

/* Current prompt string */
char g_currentPrompt[MAX_PROMPT_SIZE] = "C:\\> ";

/* Track where user input starts (after prompt) */
int g_promptEndPos = 0;

/* Forward declarations */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OutputAreaProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateControls(HWND hwnd);
void HandleRunCommand(HWND hwnd);
void HandleMicButton(HWND hwnd);
void AppendToOutput(const char *text);
void UpdatePrompt(void);
void ShowNewPrompt(void);

/* Original output area window procedure */
WNDPROC g_OriginalOutputProc = NULL;

/* 
 * Backend integration points
 * These functions will be implemented by backend team
 */

/* Receives command string, writes output to buffer */
void execute_command(const char *input, char *output);

/* Gets current working directory for prompt display */
void get_current_directory(char *buffer, int size);

/*
 * Application entry point
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow)
{
    /* Enable DPI awareness to fix blurry text */
    HMODULE hUser32 = LoadLibrary("user32.dll");
    if (hUser32) {
        SetProcessDPIAwareFunc pSetProcessDPIAware = 
            (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (pSetProcessDPIAware) {
            pSetProcessDPIAware();
        }
        FreeLibrary(hUser32);
    }
    
    const char CLASS_NAME[] = "VoiceShellWindowClass";
    
    /* Register window class */
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));  /* Black background */
    
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window registration failed!", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }
    
    /* Create main window */
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Voice Enabled Command Line Interpreter",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL
    );
    
    if (hwnd == NULL) {
        MessageBox(NULL, "Window creation failed!", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }
    
    /* Enable dark mode for title bar */
    BOOL useDarkMode = TRUE;
    HMODULE hDwmapi = LoadLibrary("dwmapi.dll");
    if (hDwmapi) {
        typedef HRESULT (WINAPI *DwmSetWindowAttributeFunc)(HWND, DWORD, LPCVOID, DWORD);
        DwmSetWindowAttributeFunc pDwmSetWindowAttribute = 
            (DwmSetWindowAttributeFunc)GetProcAddress(hDwmapi, "DwmSetWindowAttribute");
        if (pDwmSetWindowAttribute) {
            pDwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
        }
        FreeLibrary(hDwmapi);
    }
    
    /* Create monospace font (Consolas) with anti-aliasing */
    hFont = CreateFont(
        20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN,
        "Consolas"
    );
    
    /* Create child controls */
    CreateControls(hwnd);
    
    /* Show window */
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    /* Display welcome message */
    AppendToOutput("Voice Enabled Command Line Interpreter\r\n");
    AppendToOutput("Ready to accept commands.\r\n\r\n");
    
    /* Display initial prompt */
    UpdatePrompt();
    ShowNewPrompt();
    
    /* Message loop */
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    /* Cleanup */
    if (hFont) {
        DeleteObject(hFont);
    }
    
    return (int)msg.wParam;
}

/*
 * Create all GUI controls
 */
void CreateControls(HWND hwnd)
{
    /* Get client area dimensions */
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;
    
    /* Layout dimensions */
    int margin = 10;
    int bottomMargin = 15;
    int buttonHeight = 30;
    int buttonWidth = 120;
    
    /* Calculate positions - output area takes most space, button at bottom */
    int buttonY = height - margin - bottomMargin - buttonHeight;
    int outputHeight = buttonY - margin - 10;
    int outputY = margin;
    
    /* Create output area (editable, multiline - acts as terminal) */
    hOutputArea = CreateWindowEx(
        0,
        "EDIT",
        "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | 
        ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
        margin, outputY,
        width - (2 * margin), outputHeight,
        hwnd,
        (HMENU)ID_OUTPUT_AREA,
        GetModuleHandle(NULL),
        NULL
    );
    
    /* Create Voice Input button at bottom */
    hMicButton = CreateWindowEx(
        0,
        "BUTTON",
        "Voice Input",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        margin, buttonY,
        buttonWidth, buttonHeight,
        hwnd,
        (HMENU)ID_MIC_BUTTON,
        GetModuleHandle(NULL),
        NULL
    );
    
    /* Apply font to all controls */
    if (hFont) {
        SendMessage(hOutputArea, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hMicButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
    
    /* Subclass output area to handle Enter key and protect prompt */
    g_OriginalOutputProc = (WNDPROC)SetWindowLongPtr(hOutputArea, GWLP_WNDPROC, (LONG_PTR)OutputAreaProc);
    
    /* Set focus to output area */
    SetFocus(hOutputArea);
}

/*
 * Window procedure - handles all window messages
 */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH hBlackBrush = NULL;
    
    switch (uMsg) {
        case WM_CREATE:
            /* Create black brush for output area background */
            hBlackBrush = CreateSolidBrush(RGB(0, 0, 0));
            return 0;
        
        case WM_SIZE:
            /* Resize controls when window is resized */
            if (hOutputArea != NULL) {
                RECT clientRect;
                GetClientRect(hwnd, &clientRect);
                int width = clientRect.right - clientRect.left;
                int height = clientRect.bottom - clientRect.top;
                
                int margin = 10;
                int bottomMargin = 15;
                int buttonHeight = 30;
                int buttonWidth = 120;
                
                int buttonY = height - margin - bottomMargin - buttonHeight;
                int outputHeight = buttonY - margin - 10;
                int outputY = margin;
                
                /* Resize and reposition controls */
                SetWindowPos(hOutputArea, NULL, margin, outputY,
                            width - (2 * margin), outputHeight, SWP_NOZORDER);
                SetWindowPos(hMicButton, NULL, margin, buttonY,
                            buttonWidth, buttonHeight, SWP_NOZORDER);
            }
            return 0;
        
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC:
            /* Set colors for output area (black background, white text) */
            if ((HWND)lParam == hOutputArea) {
                HDC hdcEdit = (HDC)wParam;
                SetTextColor(hdcEdit, RGB(255, 255, 255));  /* White text */
                SetBkColor(hdcEdit, RGB(0, 0, 0));           /* Black background */
                return (LRESULT)hBlackBrush;
            }
            break;
        
        case WM_CTLCOLORBTN:
            /* Set dark theme for buttons */
            if ((HWND)lParam == hMicButton) {
                HDC hdcButton = (HDC)wParam;
                SetTextColor(hdcButton, RGB(255, 255, 255));  /* White text */
                SetBkColor(hdcButton, RGB(40, 40, 40));       /* Dark gray background */
                return (LRESULT)hBlackBrush;
            }
            break;
        
        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED) {
                int controlID = LOWORD(wParam);
                
                /* Handle Mic button */
                if (controlID == ID_MIC_BUTTON) {
                    HandleMicButton(hwnd);
                    return 0;
                }
            }
            break;
        
        case WM_DESTROY:
            /* Cleanup */
            if (hBlackBrush) {
                DeleteObject(hBlackBrush);
            }
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/*
 * Output area subclass procedure to handle Enter key and protect history
 */
LRESULT CALLBACK OutputAreaProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_KEYDOWN && wParam == VK_RETURN) {
        /* Enter key pressed - execute command */
        HWND parent = GetParent(hwnd);
        if (parent) {
            HandleRunCommand(parent);
        }
        return 0;  /* Prevent newline from being inserted */
    }
    
    if (uMsg == WM_CHAR && wParam == VK_RETURN) {
        /* Suppress the Enter character */
        return 0;
    }
    
    /* Prevent backspace from deleting prompt */
    if (uMsg == WM_CHAR && wParam == VK_BACK) {
        int currentPos = LOWORD(SendMessage(hwnd, EM_GETSEL, 0, 0));
        if (currentPos <= g_promptEndPos) {
            return 0;  /* Block backspace in prompt area */
        }
    }
    
    /* Prevent Delete key from deleting prompt */
    if (uMsg == WM_KEYDOWN && wParam == VK_DELETE) {
        int startSel = LOWORD(SendMessage(hwnd, EM_GETSEL, 0, 0));
        int endSel = HIWORD(SendMessage(hwnd, EM_GETSEL, 0, 0));
        if (startSel < g_promptEndPos) {
            return 0;  /* Block delete if selection includes prompt area */
        }
    }
    
    /* Prevent left arrow and Home from moving into prompt area */
    if (uMsg == WM_KEYDOWN && (wParam == VK_LEFT || wParam == VK_HOME)) {
        int currentPos = LOWORD(SendMessage(hwnd, EM_GETSEL, 0, 0));
        if (wParam == VK_HOME || currentPos <= g_promptEndPos) {
            /* Move cursor to start of user input (after prompt) */
            SendMessage(hwnd, EM_SETSEL, g_promptEndPos, g_promptEndPos);
            return 0;
        }
    }
    
    /* Prevent mouse clicks and text selection in prompt area */
    if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_MOUSEMOVE) {
        /* Let the default handler process first */
        LRESULT result = CallWindowProc(g_OriginalOutputProc, hwnd, uMsg, wParam, lParam);
        
        /* Then correct the cursor position if it's before the prompt */
        int startSel = LOWORD(SendMessage(hwnd, EM_GETSEL, 0, 0));
        int endSel = HIWORD(SendMessage(hwnd, EM_GETSEL, 0, 0));
        
        if (startSel < g_promptEndPos) {
            SendMessage(hwnd, EM_SETSEL, g_promptEndPos, 
                       (endSel < g_promptEndPos) ? g_promptEndPos : endSel);
        }
        
        return result;
    }
    
    /* Call original window procedure */
    return CallWindowProc(g_OriginalOutputProc, hwnd, uMsg, wParam, lParam);
}

/*
 * Handle Run button click
 * Gets command from output area after prompt, calls backend, displays output
 */
void HandleRunCommand(HWND hwnd)
{
    char input[MAX_INPUT_SIZE];
    char output[MAX_COMMAND_OUTPUT];
    char allText[MAX_OUTPUT_SIZE];
    int totalLength, cmdLength;
    
    /* Get total text length */
    totalLength = GetWindowTextLength(hOutputArea);
    
    /* Extract command from after the prompt */
    cmdLength = totalLength - g_promptEndPos;
    if (cmdLength <= 0) {
        /* Empty command - just show new prompt */
        ShowNewPrompt();
        return;
    }
    
    if (cmdLength >= MAX_INPUT_SIZE) {
        cmdLength = MAX_INPUT_SIZE - 1;
    }
    
    /* Get all text from output area */
    GetWindowText(hOutputArea, allText, MAX_OUTPUT_SIZE);
    
    /* Extract command portion (after prompt) */
    strncpy(input, allText + g_promptEndPos, cmdLength);
    input[cmdLength] = '\0';
    
    /* Trim whitespace */
    char *cmd = input;
    while (*cmd == ' ' || *cmd == '\t') cmd++;
    
    /* Move to end and add newline after command */
    SendMessage(hOutputArea, EM_SETSEL, totalLength, totalLength);
    SendMessage(hOutputArea, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
    
    /* Ignore empty commands */
    if (strlen(cmd) == 0) {
        ShowNewPrompt();
        return;
    }
    
    /* Call backend command execution function */
    memset(output, 0, MAX_COMMAND_OUTPUT);
    execute_command(cmd, output);
    
    /* Check if output contains clear screen marker */
    if (strstr(output, "::CLEAR_SCREEN::") != NULL) {
        /* Clear the output area */
        SetWindowText(hOutputArea, "");
        g_promptEndPos = 0;
        
        /* Update prompt and show it */
        UpdatePrompt();
        ShowNewPrompt();
        return;
    }
    
    /* Display command output */
    if (strlen(output) > 0) {
        AppendToOutput(output);
    }
    
    /* Update prompt with new directory (may have changed after cd, etc.) */
    UpdatePrompt();
    
    /* Show new prompt */
    ShowNewPrompt();
}

/*
 * Handle Mic button click
 * Placeholder for voice input functionality
 */
void HandleMicButton(HWND hwnd)
{
    /* TODO: Voice recognition integration
     * This will be connected to Python Whisper backend
     * For now, just show a placeholder message
     */
    AppendToOutput("[Voice input - awaiting backend integration]\r\n");
    ShowNewPrompt();
}

/*
 * Append text to output area and auto-scroll to bottom
 */
void AppendToOutput(const char *text)
{
    if (hOutputArea == NULL) {
        return;
    }
    
    /* Get current text length */
    int textLength = GetWindowTextLength(hOutputArea);
    
    /* Set selection to end */
    SendMessage(hOutputArea, EM_SETSEL, textLength, textLength);
    
    /* Replace selection with new text (appends) */
    SendMessage(hOutputArea, EM_REPLACESEL, FALSE, (LPARAM)text);
    
    /* Scroll to bottom */
    SendMessage(hOutputArea, EM_SCROLLCARET, 0, 0);
}

/*
 * Update prompt display with current directory
 */
void UpdatePrompt(void)
{
    char dirBuffer[MAX_PROMPT_SIZE];
    
    /* Try to get current directory from backend */
    get_current_directory(dirBuffer, MAX_PROMPT_SIZE);
    
    /* Format prompt as "directory> " */
    snprintf(g_currentPrompt, MAX_PROMPT_SIZE, "%s> ", dirBuffer);
}

/*
 * Show new prompt and set cursor position
 */
void ShowNewPrompt(void)
{
    /* Append prompt to output area */
    AppendToOutput(g_currentPrompt);
    
    /* Remember where user input starts */
    g_promptEndPos = GetWindowTextLength(hOutputArea);
    
    /* Set focus to output area */
    SetFocus(hOutputArea);
    
    /* Move cursor to end */
    SendMessage(hOutputArea, EM_SETSEL, g_promptEndPos, g_promptEndPos);
}
