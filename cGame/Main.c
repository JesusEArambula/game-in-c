#pragma warning(push, 3)

#include <stdio.h>

#include <windows.h>

#include <psapi.h>

#include <emmintrin.h>

#pragma warning(pop)

#include <stdint.h>

#include "Main.h"

#pragma comment(lib, "Winmm.lib")

HWND gGameWindow;

BOOL gGameIsRunning;

GAMEBITMAP gBackBuffer;

GAME_PERFORMANCE_DATA gPerformanceData;

PLAYER gPlayer;

BOOL gWindowHasFocus;

// Main window function
// Game loop in here
INT __stdcall WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, PSTR CommandLine, INT CmdShow)
{
    UNREFERENCED_PARAMETER(Instance);

    UNREFERENCED_PARAMETER(PreviousInstance);

    UNREFERENCED_PARAMETER(CommandLine);

    UNREFERENCED_PARAMETER(CmdShow);

    MSG Message = { 0 };

    int64_t FrameStart = 0;

    int64_t FrameEnd = 0;

    int64_t ElapsedMicroSeconds;

    int64_t ElapsedMicroSecondsPerFrameAccumulatorRaw = 0;

    int64_t ElapsedMicroSecondsPerFrameAccumulatorCooked = 0;

    HMODULE NtDllModuleHandle = NULL;

    FILETIME ProcessCreationTime = { 0 };

    FILETIME ProcessExitTime = { 0 };

    int64_t CurrentUserCPUTime = 0;

    int64_t CurrentKernelCPUTime = 0;

    int64_t PreviousUserCPUTime = 0;

    int64_t PreviousKernelCPUTime = 0;

    HANDLE ProcessHandle = GetCurrentProcess();

    if ((NtDllModuleHandle = GetModuleHandleA("ntdll.dll")) == NULL)
    { 
        MessageBoxA(NULL, "Couldn't load ntdll.dll", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((NtQueryTimerResolution = (_NtQueryTimerResolution)GetProcAddress(NtDllModuleHandle, "NtQueryTimerResolution")) == NULL)
    {
        MessageBoxA(NULL, "Couldn't find the NtQueryTimerResolution function in ntdll.dll", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    NtQueryTimerResolution(&gPerformanceData.MinimumTimerResolution, &gPerformanceData.MaximumTimerResolution, &gPerformanceData.CurrentTimerResolution);

    GetSystemInfo(&gPerformanceData.PreviousSystemTime);

    GetSystemTimeAsFileTime((FILETIME*)&gPerformanceData.CurrentSystemTime);

    if (GameIsAlreadyRunning() == TRUE)
    {
        MessageBoxA(NULL, "Another instance of thise game is already running!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if (timeBeginPeriod(1) == TIMERR_NOCANDO)
    {
        MessageBoxA(NULL, "Failed to set global timer resolution!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if (SetPriorityClass(ProcessHandle, HIGH_PRIORITY_CLASS) == 0)
    {
        MessageBoxA(NULL, "Failed to set process priority!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if (SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST) == 0)
    {
        MessageBoxA(NULL, "Failed to set thread priority!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if (CreateMainGameWindow() != ERROR_SUCCESS)
    {
        goto Exit;
    }

    QueryPerformanceFrequency((LARGE_INTEGER*)&gPerformanceData.PerformanceFrequency);  // only needs to be called once

    gPerformanceData.DisplayDebugInfo = TRUE;


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

    gPlayer.ScreenPosX = 25;

    gPlayer.ScreenPosY = 25;

    gGameIsRunning = TRUE;

    while (gGameIsRunning == TRUE)  // main game loop
    {
        QueryPerformanceCounter((LARGE_INTEGER*) &FrameStart);  // start the counter

        while (PeekMessageA(&Message, gGameWindow, 0, 0, PM_REMOVE))
        {
            DispatchMessageA(&Message);
        }

        ProcessPlayerInput();

        RenderFrameGraphics();

        QueryPerformanceCounter((LARGE_INTEGER*) &FrameEnd);    // marks end counter

        ElapsedMicroSeconds = FrameEnd - FrameStart;

        ElapsedMicroSeconds *= 1000000;

        ElapsedMicroSeconds /= gPerformanceData.PerformanceFrequency;

        gPerformanceData.TotalFramesRendered++;

        ElapsedMicroSecondsPerFrameAccumulatorRaw += ElapsedMicroSeconds;

        while (ElapsedMicroSeconds < TARGET_MICROSECONDS_PER_FRAME)
        {
            ElapsedMicroSeconds = FrameEnd - FrameStart;

            ElapsedMicroSeconds *= 1000000;

            ElapsedMicroSeconds /= gPerformanceData.PerformanceFrequency;

            QueryPerformanceCounter((LARGE_INTEGER*)&FrameEnd);

            if (ElapsedMicroSeconds < (TARGET_MICROSECONDS_PER_FRAME * 0.75f))
            {
                Sleep(1);   // anywhere from 1 millisecond to a full system timer tick (?)
            }
        }

        ElapsedMicroSecondsPerFrameAccumulatorCooked += ElapsedMicroSeconds;

        if ((gPerformanceData.TotalFramesRendered % AVG_FPS_X_FRAME) == 0)
        {
            GetSystemTimeAsFileTime((FILETIME*)&gPerformanceData.CurrentSystemTime);

            GetProcessTimes(ProcessHandle,
                &ProcessCreationTime, 
                &ProcessExitTime,
                (FILETIME*)&CurrentKernelCPUTime,
                (FILETIME*)&CurrentUserCPUTime);

            gPerformanceData.CPUPercent = (CurrentKernelCPUTime - PreviousKernelCPUTime) + (CurrentUserCPUTime - PreviousUserCPUTime);

            gPerformanceData.CPUPercent /= (gPerformanceData.CurrentSystemTime - gPerformanceData.PreviousSystemTime);

            gPerformanceData.CPUPercent /= gPerformanceData.SystemInfo.dwNumberOfProcessors;

            gPerformanceData.CPUPercent *= 100;

            GetProcessHandleCount(ProcessHandle, &gPerformanceData.HandleCount);

            GetProcessMemoryInfo(ProcessHandle, (PROCESS_MEMORY_COUNTERS*)&gPerformanceData.MemInfo, sizeof(gPerformanceData.MemInfo));

            gPerformanceData.RawFPSAverage = 1.0f / ((ElapsedMicroSecondsPerFrameAccumulatorRaw / AVG_FPS_X_FRAME) * .000001f);

            gPerformanceData.CookedFPSAverage = 1.0f / ((ElapsedMicroSecondsPerFrameAccumulatorCooked / AVG_FPS_X_FRAME) * .000001f);

            ElapsedMicroSecondsPerFrameAccumulatorRaw = 0;

            ElapsedMicroSecondsPerFrameAccumulatorCooked = 0;

            PreviousKernelCPUTime = CurrentKernelCPUTime;

            PreviousUserCPUTime = CurrentUserCPUTime;

            gPerformanceData.PreviousSystemTime = gPerformanceData.CurrentSystemTime;

        }
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

    switch (Message)
    {
        case WM_CLOSE:
        {
            gGameIsRunning = FALSE;


            PostQuitMessage(0);

            break;
        }
        case WM_ACTIVATE:
        {  
            if (WParam == 0)
            {
                gWindowHasFocus = FALSE;
            }
            else
            {
                ShowCursor(FALSE);

                gWindowHasFocus = TRUE;
            }

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

    // This function doesn't work for Windows 7 and older
    //SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    

    // Returns if Class Registration fails
    if (RegisterClassExA(&WindowClass) == 0)
    {
        Result = GetLastError();

        MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    // Window Handle
    gGameWindow = CreateWindowExA(
        0,
        WindowClass.lpszClassName,
        GAME_NAME,
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

    gPerformanceData.MonitorInfo.cbSize = sizeof(MONITORINFO);

    if (GetMonitorInfoA(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &gPerformanceData.MonitorInfo) == 0)
    {
        Result = ERROR_MONITOR_NO_DESCRIPTOR;

        goto Exit;
    }

    gPerformanceData.MonitorWidth = gPerformanceData.MonitorInfo.rcMonitor.right - gPerformanceData.MonitorInfo.rcMonitor.left;

    gPerformanceData.MonitorHeight = gPerformanceData.MonitorInfo.rcMonitor.bottom - gPerformanceData.MonitorInfo.rcMonitor.top;

    if (SetWindowLongPtrA(gGameWindow, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_OVERLAPPEDWINDOW) == 0)
    {
        Result = GetLastError();

        goto Exit;
    }

    if (SetWindowPos(
        gGameWindow, 
        HWND_TOP, 
        gPerformanceData.MonitorInfo.rcMonitor.left, 
        gPerformanceData.MonitorInfo.rcMonitor.top, 
        gPerformanceData.MonitorWidth, 
        gPerformanceData.MonitorHeight, 
        SWP_NOOWNERZORDER | SWP_FRAMECHANGED) == 0)
    {
        Result = GetLastError();

        goto Exit;
    }




Exit:

    return(Result);
}

BOOL GameIsAlreadyRunning(void)
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
    if (gWindowHasFocus == FALSE)
    {
        return;
    }

    int16_t EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

    int16_t DebugKeyIsDown = GetAsyncKeyState(VK_F1);

    int16_t LeftKeyIsDown = GetAsyncKeyState(VK_LEFT) | GetAsyncKeyState('A');

    int16_t RightKeyIsDown = GetAsyncKeyState(VK_RIGHT) | GetAsyncKeyState('D');

    int16_t UpKeyIsDown = GetAsyncKeyState(VK_UP) | GetAsyncKeyState('W');

    int16_t DownKeyIsDown = GetAsyncKeyState(VK_DOWN) | GetAsyncKeyState('S');

    static int16_t DebugKeyWasDown;

    static int16_t LeftKeyWasDown;

    static int16_t RightKeyWasDown;

    static int16_t UpKeyWasDown;

    static int16_t DownKeyWasDown;

    if (EscapeKeyIsDown)
    {
        SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    }
    if (DebugKeyIsDown && !DebugKeyWasDown)
    {
        gPerformanceData.DisplayDebugInfo = !gPerformanceData.DisplayDebugInfo;
    }
    // Player Movement
    if (LeftKeyIsDown)
    {
        if (gPlayer.ScreenPosX > 0)
        {
            gPlayer.ScreenPosX--;
        }
    }
    if (RightKeyIsDown)
    {
        if (gPlayer.ScreenPosX < GAME_WIDTH - 16)
        {
            gPlayer.ScreenPosX++;
        }
    }
    if (UpKeyIsDown)
    {
        if (gPlayer.ScreenPosY > 0)
        {
            gPlayer.ScreenPosY--;
        }
    }
    if (DownKeyIsDown)
    {
        if (gPlayer.ScreenPosY < GAME_HEIGHT - 16)
        {
            gPlayer.ScreenPosY++;
        }
    }

    DebugKeyWasDown = DebugKeyIsDown;

    LeftKeyWasDown = LeftKeyIsDown;

    RightKeyWasDown = RightKeyIsDown;

    UpKeyWasDown = UpKeyIsDown;

    DownKeyWasDown = DownKeyIsDown;


}

void RenderFrameGraphics(void)
{
#ifdef SIMD
    __m128i QuadPixel = { 0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff };

    ClearScreen(QuadPixel);
#else
    PIXEL32 Pixel = { 0x7f, 0x00, 0x00, 0xff };

    ClearScreen(&Pixel);
#endif

    int32_t ScreenX = gPlayer.ScreenPosX;

    int32_t ScreenY = gPlayer.ScreenPosY;

    int32_t StartingScreenPixel = (GAME_WIDTH * GAME_HEIGHT) - GAME_WIDTH - (GAME_WIDTH * ScreenY) + ScreenX;

    for (int32_t y = 0; y < 16; y++)
    {
        for (int32_t x = 0; x < 16; x++)
        {
            memset((PIXEL32*)gBackBuffer.Memory + (uintptr_t) StartingScreenPixel + x - ((uintptr_t) GAME_WIDTH * y), 0xFF, sizeof(PIXEL32));
        }
    }

    HDC DeviceContext = GetDC(gGameWindow);

    StretchDIBits(DeviceContext, 
        0, 
        0, 
        gPerformanceData.MonitorWidth, 
        gPerformanceData.MonitorHeight, 
        0, 
        0, 
        GAME_WIDTH, 
        GAME_HEIGHT, 
        gBackBuffer.Memory,
        &gBackBuffer.BitmapInfo, 
        DIB_RGB_COLORS, 
        SRCCOPY);

    if (gPerformanceData.DisplayDebugInfo == TRUE)
    {
        SelectObject(DeviceContext, (HFONT)GetStockObject(ANSI_FIXED_FONT));

        char DebugTextBuffer[64] = { 0 };

        // Show FPS info

        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "FPS Raw:     %.01f", gPerformanceData.RawFPSAverage);

        TextOutA(DeviceContext, 0, 0, DebugTextBuffer, (int)strlen(DebugTextBuffer));

        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "FPS Cooked:  %.01f", gPerformanceData.CookedFPSAverage);

        TextOutA(DeviceContext, 0, 13, DebugTextBuffer, (int)strlen(DebugTextBuffer));

        // Show Resolution info

        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Min Timer Resolution:  %.02f", gPerformanceData.MinimumTimerResolution / 10000.0f);

        TextOutA(DeviceContext, 0, 26, DebugTextBuffer, (int)strlen(DebugTextBuffer));
        
        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Max Timer Resolution:  %.01f", gPerformanceData.MaximumTimerResolution / 10000.0f);

        TextOutA(DeviceContext, 0, 39, DebugTextBuffer, (int)strlen(DebugTextBuffer));

        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Cur Timer Resolution:  %.01f", gPerformanceData.CurrentTimerResolution / 10000.0f);

        TextOutA(DeviceContext, 0, 51, DebugTextBuffer, (int)strlen(DebugTextBuffer));

        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Handles:  %lu", gPerformanceData.HandleCount);

        TextOutA(DeviceContext, 0, 63, DebugTextBuffer, (int)strlen(DebugTextBuffer));

        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Memory:  %ul KB", gPerformanceData.MemInfo.PrivateUsage / 1024);

        TextOutA(DeviceContext, 0, 76, DebugTextBuffer, (int)strlen(DebugTextBuffer));

        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "CPU:  %.02f %%", gPerformanceData.CPUPercent);

        TextOutA(DeviceContext, 0, 89, DebugTextBuffer, (int)strlen(DebugTextBuffer));
    }

    ReleaseDC(gGameWindow, DeviceContext);
}

#ifdef SIMD
__forceinline void ClearScreen(_In_ __m128i Color)
{
    for (int x = 0; x < GAME_WIDTH * GAME_HEIGHT; x += 4)
    {
        _mm_store_si128((PIXEL32*)gBackBuffer.Memory + x, Color);
    }
}
#else

__forceinline void ClearScreen(_In_ PIXEL32* Pixel)
{
    for (int x = 0; x < GAME_WIDTH * GAME_HEIGHT; x++)
    {
        memcpy((PIXEL32*) gBackBuffer.Memory + x, Pixel, sizeof(PIXEL32));
    }
}
#endif
