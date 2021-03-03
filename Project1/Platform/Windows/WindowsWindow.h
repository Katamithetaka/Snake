#pragma once



#ifdef VK_USE_PLATFORM_WIN32_KHR

#include "Core/Window.h"
#include <vector>

namespace Mountain {


	class WindowsWindow: public Window {


	public: 

		WindowsWindow(WindowProps& props);
		~WindowsWindow() { 
			if (lastEvent) {
				delete lastEvent;
				lastEvent = NULL;
			} 
		}
		Window& SetFullscreen(bool fullscreen) override;

		virtual void Close() override;
		virtual Event* Update() override;



	private:


		WindowProps Props;
		friend HWND& GetHWND(Window* Window);
		friend HINSTANCE& GethInstance(Window* Window);
		friend LRESULT CALLBACK WndProc(HWND hwnd, UINT i, WPARAM wparam, LPARAM lparam);
		MONITORINFO FullscreenDims;
		RECT RealCoords;
		HINSTANCE hInstance;
		unsigned style = 0;
		HWND Hwnd;

	};


	HINSTANCE& GethInstance(Window* _Window) {
		WindowsWindow* WW = reinterpret_cast<WindowsWindow*>(_Window);

		return WW->hInstance;
	}

	HWND& GetHWND(Window* _Window) {
		WindowsWindow* WW = reinterpret_cast<WindowsWindow*>(_Window);

		return WW->Hwnd;
	}

	Window* CreateButton(const char* Name, unsigned x, unsigned y, unsigned width, unsigned height, Window* Parent) {
		WindowProps props;
		props.Name = Name;
		props.x = x;
		props.y = y;
		props.Width = width;
		props.Height = height;
		props.WindowCreationFlags = WINDOW_CHILD;
		assert(Parent && "Cannot create a button without a parent window");
		props.Parent = Parent;
		props.className = "Button";
		return new WindowsWindow(props);
	}




	Window* CreateDropDown(const char* Name, unsigned x, unsigned y, unsigned width, unsigned height, Window* Parent, const char** options, unsigned size) {
		DropDownProps props;

		props.Name = Name;
		props.x = x;
		props.y = y;
		props.Width = width;
		props.Height = height;
		props.WindowCreationFlags = WINDOW_CHILD;
		assert(Parent && "Cannot create a drop down without a parent window");
		props.Parent = Parent;
		props.className = "ComboBox";
		props.options = options;
		props.size = size;
		return new WindowsWindow(props);
	}



	Window* CreatePlatformWindow(WindowProps& props) {
		return new WindowsWindow(props);
	}

}
#endif