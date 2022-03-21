#pragma once

#define GAME_NAME		"Game B"

#define GAME_WIDTH		384

#define GAME_HEIGHT		240

#define GAME_BPP		32	// bits per pixel

#define GAME_CANVAS		(GAME_WIDTH * GAME_HEIGHT * (GAME_BPP / 8))

// Struct for Game Bit Map
typedef struct GAMEBITMAP
{
	BITMAPINFO BitmapInfo;

	void* Memory;

} GAMEBITMAP;

// 32 bit Pixel Structure
typedef struct PIXEL32
{
	uint8_t Blue;

	uint8_t Green;

	uint8_t Red;

	uint8_t Alpha;

} PIXEL32;



LRESULT CALLBACK MainWindowProc(
	_In_ HWND WindowHandle,
	_In_ UINT Message,
	_In_ WPARAM WParam,
	_In_ LPARAM LParam);

DWORD CreateMainGameWindow(void);

BOOL GameIsAlreadyRunning(void);

void ProcessPlayerInput(void);

void RenderFrameGraphics(void);

