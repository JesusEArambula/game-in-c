#pragma once

#define GAME_NAME			"Game B"

#define GAME_WIDTH			384

#define GAME_HEIGHT			240

#define GAME_BPP			32	// bits per pixel

#define GAME_CANVAS			(GAME_WIDTH * GAME_HEIGHT * (GAME_BPP / 8))

#define AVG_FPS_X_FRAME		100	// after ever X frames, calculate the average FPS


#pragma warning(disable: 4820)	// disable warning regarding structure padding

#pragma warning(disable: 5045)	// disable warning about Spectre vulnerability

typedef struct GAMEBITMAP	// Game Bit Map Structure
{
	BITMAPINFO BitmapInfo;

	void* Memory;

} GAMEBITMAP;

typedef struct PIXEL32		// 32 bit Pixel Structure
{
	uint8_t Blue;

	uint8_t Green;

	uint8_t Red;

	uint8_t Alpha;

} PIXEL32;

typedef struct GAME_PERFORMANCE_DATA
{
	uint64_t TotalFramesRendered;

	uint32_t RawFramesPerSecondAverage;

	uint32_t CookedFramesPerSecondAverage;

	LARGE_INTEGER PerformanceFrequency;

	LARGE_INTEGER FrameStart;

	LARGE_INTEGER FrameEnd;

	LARGE_INTEGER ElapsedMicrosecondsPerFrame;
	
	MONITORINFO MonitorInfo;

	int32_t MonitorWidth;

	int32_t MonitorHeight;

} GAME_PERFORMANCE_DATA;


LRESULT CALLBACK MainWindowProc(
	_In_ HWND WindowHandle,
	_In_ UINT Message,
	_In_ WPARAM WParam,
	_In_ LPARAM LParam);

DWORD CreateMainGameWindow(void);

BOOL GameIsAlreadyRunning(void);

void ProcessPlayerInput(void);

void RenderFrameGraphics(void);

