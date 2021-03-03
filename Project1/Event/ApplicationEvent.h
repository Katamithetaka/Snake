#pragma once

#include "Event.h"

namespace Mountain {
	// 		WindowMove, WindowResize, WindowFocus, WindowLostFocus, ApplicationUpdate

	struct WindowMoveEvent : public Event {

		WindowMoveEvent(int32_t x, int32_t y, Window* win)
			: m_X(x), m_Y(y), Event(win)
		{

		}
		
		int32_t GetX() const { return m_X; }
		int32_t GetY() const { return m_Y; }

		virtual EventTypes GetEventType() const {

			return EventTypes::WindowMove;


		}
		virtual uint8_t GetCategoryFlags() const {
			
			return EVENT_CATEGORY_APPLICATION | EVENT_CATEGORY_INPUT;
			
		}

		virtual std::string ToString() const {

			return "WindowMoveEvent : {" + std::to_string(m_X) + " , " + std::to_string(m_Y) + "}";
		}


	private:

		int32_t m_X, m_Y;
	};

	struct WindowResizeEvent : public Event {

		WindowResizeEvent(int32_t x, int32_t y, Window* win)
			: m_X(x), m_Y(y), Event(win)
		{

		}

		int32_t GetWidth() const { return m_X; }
		int32_t GetHeight() const { return m_Y; }

		virtual EventTypes GetEventType() const {

			return EventTypes::WindowResize;


		}
		virtual uint8_t GetCategoryFlags() const {

			return EVENT_CATEGORY_APPLICATION | EVENT_CATEGORY_INPUT;

		}


		virtual std::string ToString() const {

			return "WindowResizeEvent : {" + std::to_string(m_X) + " , " + std::to_string(m_Y) + "}";
		}

	private:

		int32_t m_X, m_Y;
	};

	struct WindowLostFocusEvent : public Event {

		WindowLostFocusEvent(Window* win): Event(win){}
		~WindowLostFocusEvent(){}

		virtual EventTypes GetEventType() const {

			return EventTypes::WindowLostFocus;


		}
		virtual uint8_t GetCategoryFlags() const {

			return EVENT_CATEGORY_APPLICATION;

		}


		virtual std::string ToString() const {

			return "WindowLostFocusEvent";
		}
	};

	struct WindowFocusEvent : public Event {

		WindowFocusEvent(Window* win): Event(win) {}
		~WindowFocusEvent() {}

		virtual EventTypes GetEventType() const {

			return EventTypes::WindowFocus;


		}
		virtual uint8_t GetCategoryFlags() const {

			return EVENT_CATEGORY_APPLICATION;

		}


		virtual std::string ToString() const {

			return "WindowFocusEvent";
		}
	};

	struct WindowCloseEvent : public Event {

		WindowCloseEvent(Window* win): Event(win) {}
		~WindowCloseEvent() {}

		virtual EventTypes GetEventType() const {

			return EventTypes::WindowClose;


		}
		virtual uint8_t GetCategoryFlags() const {

			return EVENT_CATEGORY_APPLICATION;

		}


		virtual std::string ToString() const {

			return "WindowCloseEvent";
		}
	};

	struct ApplicationUpdateEvent : public Event {

		ApplicationUpdateEvent(float delta , Window* win)
			: m_DeltaTime(delta), Event(win)
		{

		}
		
		bool HasDeltaTime() const { return m_DeltaTime != -1; }
		// Should test with HasDeltaTime() before calling GetDeltaTime() to know if deltatime was provided.
		float GetDeltaTime() const { return m_DeltaTime; }


		virtual EventTypes GetEventType() const {

			return EventTypes::ApplicationUpdate;


		}
		virtual uint8_t GetCategoryFlags() const {

			return EVENT_CATEGORY_APPLICATION;

		}


		virtual std::string ToString() const {

			return "ApplicationUpdateEvent : DeltaTime = " + std::to_string(m_DeltaTime);
		}
	private:
		float m_DeltaTime;
	};
}