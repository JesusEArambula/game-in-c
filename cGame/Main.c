#include <stdio.h>

#pragma warning(push, 3)

#include <windows.h>

#pragma warning(pop)

#include <stdint.h>

#include "Main.h"

HWND gGameWindow;

BOOL gGameIsRunning;

GAMEBITMAP gBackBuffer;

RECT gGameWindowSize;

MONITORINFO gMonitorInfo = { sizeof(MONITORINFO) };


int WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, PSTR CommandLine, INT CmdShow)
{
    UNREFERENCED_PARAMETER(Instance);

    UNREFERENCED_PARAMETER(PreviousInstance);

    UNREFERENCED_PARAMETER(CommandLine);

    UNREFERENCED_PARAMETER(CmdShow);

    if (GameIsAlreadyRunning() == TRUE)
    {
        MessageBox(NULL, "Another instance of thise game is already running!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if (CreateMainGameWindow() != ERROR_SUCCESS)
    {
        goto Exit;
    }

    gBackBuffer.BitmapInfo.bmiHeader.biSize = sizeof(gBackBuffer.BitmapInfo.bmiHeader);

    gBackBuffer.BitmapInfo.bmiHeader.biWidth = GAME_WIDTH;

    gBackBuffer.BitmapInfo.bmiHeader.biHeight = GAME_HEIGHT;

    gBackBuffer.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;

    gBackBuffer.BitmapInfo.bmiHeader.biCompression = BI_RGB;

    gBackBuffer.BitmapInfo.bmiHeader.biPlanes = 1;

    gBackBuffer.Memory = VirtualAlloc(NULL, GAME_CANVAS, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (gBackBuffer.Memory == NULL)
    {
        MessageBoxA(NULL, "Failed to allocate memory for drawing surface!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    memset(gBackBuffer.Memory, 0x7F, GAME_CANVAS);


    MSG Message = { 0 };

    gGameIsRunning = TRUE;

    while (gGameIsRunning)
    {
        // Message (main) Loop
        while (PeekMessageA(&Message, gGameWindow, 0, 0, PM_REMOVE))
        {
            // TranslateMessage(&Message);

            DispatchMessageA(&Message);
        }

        ProcessPlayerInput();

        RenderFrameGraphics();

        Sleep(1);
    }


Exit:

    return(0);
}

LRESULT CALLBACK MainWindowProc(
    _In_ HWND WindowHandle,  // handle to window
    _In_ UINT Message,       // message identifier
    _In_ WPARAM WParam,      // first message parameter
    _In_ LPARAM LParam)      // second message parameter
{
    LRESULT Result = 0;

    // See the code messages from interacting with window
    /*
    char buf[12] = { 0 };

    _itoa_s(Message, buf, _countof(buf), 10);

    OutputDebugStringA(buf);

    OutputDebugStringA("\n");
    */

    switch (Message)
    {
    case WM_CLOSE:
    {
        gGameIsRunning = FALSE;


        PostQuitMessage(0);

        break;
    }
    default:
    {
        Result = DefWindowProcA(WindowHandle, Message, WParam, LParam);
    }
    }
    return(Result);
}

DWORD CreateMainGameWindow(void)
{
    DWORD Result = ERROR_SUCCESS;

    // Making a type of window to show
    WNDCLASSEXA WindowClass = { 0 };

    WindowClass.cbSize = sizeof(WNDCLASSEXA);

    // Will use style to make game full screen
    WindowClass.style = 0;

    WindowClass.lpfnWndProc = MainWindowProc;

    WindowClass.cbClsExtra = 0;

    WindowClass.cbWndExtra = 0;

    WindowClass.hInstance = GetModuleHandleA(NULL);

    WindowClass.hIcon = LoadIconA(NULL, IDI_APPLICATION);

    WindowClass.hIconSm = LoadIconA(NULL, IDI_APPLICATION);

    WindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);

    WindowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));

    WindowClass.lpszMenuName = NULL;

    WindowClass.lpszClassName = GAME_NAME "_WINDOWCLASS";


    // Returns if Class Registration fails
    if (RegisterClassExA(&WindowClass) == 0)
    {
        Result = GetLastError();

        MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    // Window Handle
    gGameWindow = CreateWindowExA(
        WS_EX_CLIENTEDGE,
        WindowClass.lpszClassName,
        "Title of Window",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        640,
        480,
        NULL,
        NULL,
        GetModuleHandleA(NULL),
        NULL
    );

    // Return if Window Handle fails
    if (gGameWindow == NULL)
    {
        Result = GetLastError();

        MessageBoxA(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if (GetMonitorInfo(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &gMonitorInfo) == 0)
    {
        Result = ERROR_MONITOR_NO_DESCRIPTOR;

        goto Exit;
    }

    int MonitorWidth = gMonitorInfo.rcMonitor.right - gMonitorInfo.rcMonitor.left;

    int MonitorHeight = gMonitorInfo.rcMonitor.bottom - gMonitorInfo.rcMonitor.top;


Exit:

    return(Result);
}

BOOL GameIsAlreadyRunning()
{
    HANDLE Mutex = NULL;

    Mutex = CreateMutexA(NULL, FALSE, GAME_NAME "_GameMutex");

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

void ProcessPlayerInput(void)
{
    SHORT EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

    if (EscapeKeyIsDown)
    {
        SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    }

}

void RenderFrameGraphics(void)
{
    HDC DeviceContext = GetDC(gGameWindow);

    StretchDIBits(DeviceContext, 0, 0, 100, 100, 0, 0, 100, 100, gBackBuffer.Memory, &gBackBuffer.BitmapInfo, DIB_RGB_COLORS, SRCCOPY);



    ReleaseDC(gGameWindow, DeviceContext);
}