#include "Core/Platform.h"
#ifdef VK_USE_PLATFORM_WIN32_KHR

// Don't question it, it just makes buttons and dropdown look pretty.
#pragma comment(linker,"\"/manifestdependency:type='win32' \
	name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
	processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "Event/ApplicationEvent.h"

#include "WindowsWindow.h"
#include <string>
#include <iterator>
#include <cstdio>
#include <functional>

namespace Mountain {




	 LRESULT CALLBACK WndProc(HWND hwnd, UINT i, WPARAM wparam, LPARAM lparam) {
		 
		 WindowsWindow* win = reinterpret_cast<WindowsWindow*>(GetPropA(hwnd, "Class"));

		 switch (i) {
			

			 case WM_CLOSE:
				 if (win->lastEvent) delete win->lastEvent;
				 win->lastEvent = new WindowCloseEvent(win);
				 return 0;
		 }

		 return DefWindowProc(hwnd, i, wparam, lparam);

	}

	 void GenWNDCLASSEX(WindowProps& props, HINSTANCE& hInstance) {
		 WNDCLASSEX wc;

		 wc.cbSize = sizeof(WNDCLASSEX);
		 wc.lpszClassName = props.className;
		 wc.style = 0;
		 wc.lpszMenuName = NULL;
		 wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		 wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		 wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		 wc.lpfnWndProc = WndProc;
		 wc.cbClsExtra = 0;
		 wc.cbWndExtra = sizeof(LONG_PTR);
		 wc.hInstance = hInstance;
		 wc.hIconSm = NULL;

		 if (!RegisterClassEx(&wc)) {

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

			 printf_s("Error code: %d, Error Message: %s\n", dw, (char*)lpMsgBuf);

			 assert(!"Failed to register window class." );
		 }

	 }
 

	 WindowsWindow::WindowsWindow(WindowProps& props) : Props(props), FullscreenDims{}, Hwnd{} {

		 style |= WS_VISIBLE ;
		 if (Props.WindowCreationFlags & WINDOW_CHILD) {
			 style |= WS_CHILD;
			 if (Props.className == "ComboBox") {
				 style |= CBS_DROPDOWN | CBS_HASSTRINGS;
				 hInstance = reinterpret_cast<WindowsWindow*>((props.Parent))->hInstance;
				 DropDownProps* dpProps = reinterpret_cast<DropDownProps*>(&props);
				 HWND Parent;

				 if (!(Parent = GetHWND(props.Parent))) {
					 assert(!"Parent doesn't have a HWND");
				 }

				 Hwnd = CreateWindowEx(0,
					 Props.className,
					 Props.Name,
					 style | CBS_DROPDOWNLIST,
					 Props.x, Props.y, Props.Width, Props.Height,
					 Parent, 0, hInstance, 0);
				 // Segoe UI
			 
				 if (!Hwnd) {

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

					 printf_s("Error code: %d, Error Message: %s\n", dw, (char*)lpMsgBuf);

					 assert(!"Failed to create dropdown");
				 }

				 ShowWindow(Hwnd, SW_SHOW);
				 for (unsigned i = 0; i < dpProps->size; ++i) {
					 SendMessage(Hwnd, CB_ADDSTRING, 0, LPARAM(dpProps->options[i]));
				 }

			 }
			 else if (Props.className == "Button") {
				 style |= BS_DEFPUSHBUTTON;

				 hInstance = reinterpret_cast<WindowsWindow*>((props.Parent))->hInstance;
				 DropDownProps* dpProps = reinterpret_cast<DropDownProps*>(&props);
				 HWND Parent;

				 if (!(Parent = GetHWND(props.Parent))) {
					 assert(!"Parent doesn't have a HWND");
				 }

				 Hwnd = CreateWindowEx(0,
					 Props.className,
					 Props.Name,
					 style,
					 Props.x, Props.y, Props.Width, Props.Height,
					 Parent, 0, hInstance, 0);

				 if (!Hwnd) {

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

					 printf_s("Error code: %d, Error Message: %s\n", dw, (char*)lpMsgBuf);

					 assert(!"Failed to create dropdown");
				 }



			 }
		 }
		 else {

			 

			 style |= WS_OVERLAPPED | WS_CAPTION | WS_VISIBLE | WS_SYSMENU;
			 if (Props.WindowCreationFlags & WINDOW_RESIZABLE) {
				 style |= WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
			 }
			 if (props.WindowCreationFlags & WINDOW_FULLSCREEN) {
				 SetFullscreen(true);
			 }


			 if (!Hwnd) {
				 Props.className = Props.className == "" ? Props.Name : Props.className;
				 
				 GenWNDCLASSEX(Props, hInstance);
				 Hwnd = CreateWindowExA(0,
					 Props.className,
					 props.Name,
					 style,
					 Props.x, Props.y, Props.Width, Props.Height,
					 NULL, NULL, hInstance, NULL);



				 if (!Hwnd) {
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

					 printf_s("Error code: %d, Error Message: %s\n", dw, (char*)lpMsgBuf);

					 assert(!"Failed to create window");
				 }
			 }

			 if (props.WindowCreationFlags & WINDOW_VULKAN_CONTEXT) {

				 //VkWin32SurfaceCreateInfoKHR create_info{};
				 //create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
				 //create_info.hinstance = hInstance;
				 //create_info.hwnd = Hwnd;
				 //vkCreateWin32SurfaceKHR(_renderer->GetVulkanInstance(), &create_info, nullptr, &_surface);

			 }
		 }

		 
		 if (Hwnd) {
			 SetPropA(Hwnd, "Class", this);
			 ShowWindow(Hwnd, SW_SHOW);
		 }
	}

	Window& WindowsWindow::SetFullscreen(bool fullscreen) {
	
		assert(Props.WindowCreationFlags & WINDOW_CHILD && "Child window cannot be fullscreen (yet)");
		if (fullscreen) {
			FullscreenDims = { sizeof(FullscreenDims) };
	
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
		
			Hwnd = CreateWindowEx(0,
				Props.Name,
				Props.Name,
				0,
				Props.x, Props.y, Props.Width, Props.Height,
				0, NULL, 0, 0);

			Props.WindowCreationFlags &= (1 << (WINDOW_FULLSCREEN / 2));

		}

		return *this;
	}

	bool CALLBACK SetFont(HWND child, LPARAM font) {
		SendMessage(child, WM_SETFONT, font, true);
		return true;
	}


	Event* WindowsWindow::Update() {
		MSG msg;
		BOOL bRet;
		static bool hasSetFont = false;
	
		if (!hasSetFont) {
			HFONT font = CreateFont(
				18, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, ANSI_CHARSET,
				OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
				DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
			SendMessage(Hwnd, WM_SETFONT, (LPARAM)font, true);
			EnumChildWindows(Hwnd, (WNDENUMPROC)SetFont, (LPARAM)font);
			hasSetFont = true;
		}

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

}
#endif