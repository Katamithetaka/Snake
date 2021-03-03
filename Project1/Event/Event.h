#pragma once

#include <string>

#include "Core/Window.h"

namespace Mountain {


	enum class EventTypes : uint8_t
	{
		None,
		MouseButtonPress, MouseButtonRelease, MouseMove, MouseScroll,
		KeyPress, KeyRelease,
		WindowMove, WindowResize, WindowFocus, WindowLostFocus, WindowClose, ApplicationUpdate

	};

	enum EventCategory
	{
		EVENT_CATEGORY_MOUSE		= 0x01,
		EVENT_CATEGORY_INPUT		= 0x02,
		EVENT_CATEGORY_MOUSE_BUTTON = 0x04,
		EVENT_CATEGORY_KEYBOARD		= 0x08,
		EVENT_CATEGORY_APPLICATION	= 0x10,
	};


	struct Event {


		virtual ~Event() {}
		
		virtual EventTypes GetEventType() const {
			return EventTypes::None;
		}

		virtual uint8_t GetCategoryFlags() const = 0;
		
		
		virtual std::string ToString() const  = 0;

		uint8_t IsInCategory(uint8_t EventCategoryFlag) { return GetCategoryFlags() & EventCategoryFlag; }
	
		Window* m_Window;

	protected:
		Event(Window* win): m_Window(win) 
		{}

	};

}
