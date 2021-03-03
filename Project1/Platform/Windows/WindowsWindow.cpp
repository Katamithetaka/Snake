#include "Core/Platform.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR

// Don't question it, it just makes buttons and dropdown look pretty.
#pragma comment(linker,"\"/manifestdependency:type='win32' \
	name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
	processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "Event/ApplicationEvent.h"
#include "Event/MouseEvent.h"
#include "WindowsWindow.h"
#include <string>
#include <iterator>
#include <cstdio>
#include <functional>

namespace Mountain {




	 LRESULT CALLBACK WndProc(HWND hwnd, UINT i, WPARAM wparam, LPARAM lparam) {
		 
		 WindowsWindow* win = reinterpret_cast<WindowsWindow*>(GetPropA(hwnd, "Class"));


		 if (i == WM_PARENTNOTIFY) {
			 switch (LOWORD(wparam)) {
				 case WM_CREATE: {
					HFONT font = CreateFont(
						18, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, ANSI_CHARSET,
						OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
						DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
					SendMessage((HWND)lparam, WM_SETFONT, WPARAM(font), true);
					break;
				 }
			 }
		 }
		 switch (i) {
			case WM_COMMAND: {
				if (win->lastEvent) {
					delete win->lastEvent;
					win->lastEvent = 0;
				}
				win->lastEvent = new ChildEvent(win, reinterpret_cast<WindowsWindow*>(GetPropA(HWND(lparam), "Class")));
				return 0;
			}
			case WM_CLOSE:
				if (win->lastEvent) {
					delete win->lastEvent;
					win->lastEvent = 0;
				}
				win->lastEvent = new WindowCloseEvent(win);
				return 0;
		 }

		 return DefWindowProc(hwnd, i, wparam, lparam);

	}

	 void WindowsWindow::_GenWindowClass() {
		 WNDCLASSEX wc;
		 if (Props.className == "") Props.className = Props.Name;
		 wc.cbSize = sizeof(WNDCLASSEX);
		 wc.lpszClassName = Props.className;
		 wc.style = CS_HREDRAW | CS_VREDRAW;
		 wc.lpszMenuName = NULL;
		 wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		 wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		 wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		 wc.lpfnWndProc = WndProc;
		 wc.cbClsExtra = 0;
		 wc.cbWndExtra = sizeof(LONG_PTR);
		 wc.hInstance = hInstance;
		 wc.hIconSm = NULL;



		 _CheckError(RegisterClassEx(&wc), "Failed to create Window Class");
	 }

	 void WindowsWindow::_CheckError(bool condition, const char* message) {
		 if (!condition) {
			 LPVOID lpMsgBuf;
			 LPVOID lpDisplayBuf;
			 DWORD dw = GetLastError();
			 FormatMessage(
				 FORMAT_MESSAGE_ALLOCATE_BUFFER |
				 FORMAT_MESSAGE_FROM_SYSTEM |
				 FORMAT_MESSAGE_IGNORE_INSERTS,
				 NULL,
				 dw,
				 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				 (LPTSTR)&lpMsgBuf,
				 0, NULL);

			 lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
				 (lstrlen((LPCTSTR)lpMsgBuf) * sizeof(TCHAR)));

			 printf_s("Error code: %d,\n Error Message: %s", dw, (char*)lpMsgBuf);

			 printf_s("%s \n", message);
			 assert(!"Error!");
		 }
	 }

	 void WindowsWindow::_CreateWindow() {
		 HWND Parent = Props.Parent ? GetHWND(Props.Parent) : nullptr;
		 if (!Parent && Props.WindowCreationFlags & WINDOW_CHILD) assert(!Parent && "Child window must have a Parent!");
		 if (Props.Parent && Props.Parent->Props.WindowCreationFlags & WINDOW_VULKAN_CONTEXT) assert(!"Vulkan windows can't have children!");
		 if (Parent) hInstance = reinterpret_cast<WindowsWindow*>(Props.Parent)->hInstance;
		 RECT Rect = { 0, 0, LONG(Props.Width), LONG(Props.Height) };
		 AdjustWindowRectEx(&Rect, style, FALSE, 0);

		 Hwnd = CreateWindowEx(0,
			 Props.className,
			 Props.Name,
			 style,
			 Props.x, Props.y, Rect.right - Rect.left, Rect.bottom - Rect.top,
			 Parent, 0, hInstance, 0);

		 _CheckError(Hwnd, "Failed to create Window");

		 SetPropA(Hwnd, "Class", this);
		 ShowWindow(Hwnd, SW_SHOW);
	 }
 
	 void WindowsWindow::_MakeStyle() {
		 style |= WS_VISIBLE;

		 if (Props.WindowCreationFlags & WINDOW_CHILD) {
			 if (Props.className == "ComboBox")  style |= CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD;
			 else if(Props.className == "Button") style |= BS_PUSHBUTTON | WS_CHILD;
		 }
		 else {
			 style |= WS_OVERLAPPED | WS_CAPTION | WS_VISIBLE | WS_SYSMENU | WS_CLIPCHILDREN;
			
			 if (Props.WindowCreationFlags & WINDOW_RESIZABLE) {
				 style |= WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
			 }
			 
			 if (Props.WindowCreationFlags & WINDOW_FULLSCREEN) {
				 SetFullscreen(true);
			 }
		 }
	 }

	 WindowsWindow::WindowsWindow(WindowProps& props) : Hwnd{}, hInstance{} {
		 
		 Props = props;
		 _MakeStyle();
		 if (Props.WindowCreationFlags & WINDOW_CHILD) {
			 if (Props.className == "ComboBox") {

				 hInstance = reinterpret_cast<WindowsWindow*>((props.Parent))->hInstance;
				 DropDownProps* dpProps = reinterpret_cast<DropDownProps*>(&props);

				 _CreateWindow();
				 
				 if (Hwnd != 0) {
					 for (unsigned i = 0; i < dpProps->size; ++i) {
						 SendMessage(Hwnd, CB_ADDSTRING, 0, LPARAM(dpProps->options[i]));
					 }
				 }

			 }
			 else if (Props.className == "Button") {

				 hInstance = reinterpret_cast<WindowsWindow*>((props.Parent))->hInstance;

				 _CreateWindow();

			 }
		 }
		 else {
			 if (!Hwnd) {

				 _GenWindowClass();

				 _CreateWindow();
			 }
		 }
	}

	Window& WindowsWindow::SetFullscreen(bool fullscreen) {
	
		assert(Props.WindowCreationFlags & WINDOW_CHILD && "Child window cannot be fullscreen!");
		if (fullscreen) {
			MONITORINFO FullscreenDims = { sizeof(FullscreenDims) };
	
			Hwnd = CreateWindowEx(0,
				"static",
				Props.Name,
				WS_POPUP | WS_VISIBLE,
				FullscreenDims.rcMonitor.left,
				FullscreenDims.rcMonitor.top,
				FullscreenDims.rcMonitor.right - FullscreenDims.rcMonitor.left,
				FullscreenDims.rcMonitor.bottom - FullscreenDims.rcMonitor.top,
				0, NULL, 0, 0);

			Props.WindowCreationFlags |= WINDOW_FULLSCREEN;
		}
		else {
		
			_CreateWindow();
			Props.WindowCreationFlags &= (1 << (WINDOW_FULLSCREEN / 2));

		}

		return *this;
	}



	Event* WindowsWindow::Update() {
		MSG msg;
		BOOL bRet;

		if ((bRet = GetMessage(&msg, 0, 0, 0)) != 0)
		{
			if (bRet == -1)
			{
				// handle the error and possibly exit
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		return lastEvent;
	}

	void WindowsWindow::Close() {
		assert(!(Props.WindowCreationFlags & WINDOW_CHILD) && "Child window cannot be closed.");
		CloseWindow(Hwnd);
		DestroyWindow(Hwnd);
		UnregisterClassA(Props.className, hInstance);
		m_Closed = true;
	}


	void GetDesktopResolution(unsigned& Width, unsigned& Height)
	{
		RECT desktop;
		const HWND hDesktop = GetDesktopWindow();
		GetWindowRect(hDesktop, &desktop);
		Width = desktop.right;
		Height = desktop.bottom;
	}

}
#endif