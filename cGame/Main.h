#pragma once

#define GAME_NAME		"Game B"

#define GAME_WIDTH		384

#define GAME_HEIGHT		216

#define GAME_BPP		32	// bits per pixel

#define GAME_CANVAS		(GAME_WIDTH * GAME_HEIGHT * (GAME_BPP / 8))

typedef struct GAMEBITMAP
{
	BITMAPINFO BitmapInfo;

	void* Memory;
} GAMEBITMAP;



LRESULT CALLBACK MainWindowProc(
	_In_ HWND WindowHandle, 
	_In_ UINT Message, 
	_In_ WPARAM WParam, 
	_In_ LPARAM LParam);

DWORD CreateMainGameWindow(void);

BOOL GameIsAlreadyRunning(void);

void ProcessPlayerInput(void);

void RenderFrameGraphics(void);

