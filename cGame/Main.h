#pragma once

#define GAME_NAME						"Game B"

#define GAME_WIDTH						384

#define GAME_HEIGHT						240

#define GAME_BPP						32	// bits per pixel

#define GAME_CANVAS						(GAME_WIDTH * GAME_HEIGHT * (GAME_BPP / 8))

#define AVG_FPS_X_FRAME					100	// after ever X frames, calculate the average FPS

#define TARGET_MICROSECONDS_PER_FRAME	16667

#pragma warning(disable: 4820)	// disable warning regarding structure padding

#pragma warning(disable: 5045)	// disable warning about Spectre vulnerability

typedef LONG(NTAPI* _NtQueryTimerResolution) (OUT PULONG MinimumResolution, OUT PULONG MaximumResolution, OUT PULONG CurrentResolution);

_NtQueryTimerResolution NtQueryTimerResolution;

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

	float RawFPSAverage;

	float CookedFPSAverage;

	int64_t PerformanceFrequency;

	MONITORINFO MonitorInfo;

	int32_t MonitorWidth;

	int32_t MonitorHeight;

	BOOL DisplayDebugInfo;

	LONG MinimumTimerResolution;

	LONG MaximumTimerResolution;

	LONG CurrentTimerResolution;

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

void ClearScreen(_In_ __m128i Color);
