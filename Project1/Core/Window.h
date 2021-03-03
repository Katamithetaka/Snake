#pragma once

#include "Platform.h"
#include <cassert>

namespace Mountain {

	enum WINDOW_CREATION_FLAGS {
		WINDOW_FULLSCREEN = 0b0000001,
		WINDOW_RESIZABLE = 0b0000010,
		WINDOW_VULKAN_CONTEXT = 0b0000100,
		WINDOW_CHILD = 0b0001000
	};

	struct WindowProps
	{

		// Size and positions will be ignored if window is set fullscreen
		unsigned x = 200, y = 200;

		// Size and positions will be ignored if window is set fullscreen
		unsigned Width = 720, Height = 720;

		/*
			Will be used:
				- Button Content
				- Drop Down Name
				- Window Class Name (if className isn't defined)
				- Vulkan Application Name
		*/
		const char* Name = "Waw cool name isn't it";
		const char* className = "";


		/*
			WINDOW_FULLSCREEN		= 0b0000001,
			WINDOW_RESIZABLE		= 0b0000010,
			WINDOW_VULKAN_CONTEXT	= 0b0000100,
			WINDOW_CHILD			= 0b0001000


			Notes:
				- Window Child cannot be used with any other flags
				- Window Resizable and Window fullscreen are completely incompatible
		*/
		unsigned WindowCreationFlags = 0;

		// If you don't put WINDOW_CHILD in WindowCreationFlags, parent won't be assigned even if it's part of the WindowProps instance
		struct Window* Parent = nullptr;

		WindowProps() {}

	};


	// Before creating a window, a WindowProps instance must be created and passed into the constructor.
	struct Window {

		virtual Window& SetFullscreen(bool fullscreen) = 0;

		bool IsClosed() { return m_Closed; }
		virtual void Close() = 0;

		virtual struct Event* Update() = 0;

		const char* GetName() const { return Props.Name; }
		
		WindowProps Props;

	protected:
		struct Event* lastEvent;

		friend HWND& GetHWND(Window* Window);
		bool m_Closed = false;
		friend struct Event;

	};


	struct DropDownProps : WindowProps {

		const char** options;
		unsigned size;

		DropDownProps(){}

	};

	Window* CreatePlatformWindow(WindowProps& props);
	Window* CreateButton(const char* Name, unsigned x, unsigned y, unsigned width, unsigned height, Window* Parent);
	Window* CreateDropDown(const char* Name, unsigned x, unsigned y, unsigned width, unsigned height, Window* Parent, const char** options, unsigned size);

	void GetDesktopResolution(unsigned& Width, unsigned& Height);


}

#include "Event/Event.h"
