#pragma once

#define NOMINMAX
#define WINDOW_GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define WINDOW_GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

#include <Windows.h>
#include <string>

class Window
{
public:
	HWND hwnd;
	HINSTANCE hinstance;

	bool keys[256];
	int mousex;
	int mousey;
	bool mouseButtons[3];

	Window() {}
	~Window() {}

	void create(int window_x = 0, int window_y = 0,
				int window_width = 1920, int window_height = 1080,
				const std::string& window_name = "",
				WNDPROC wndProc = nullptr);


	void updateMouse(int x, int y);

	void processMessages();
};

