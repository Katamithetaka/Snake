#pragma once 

#include "Event.h"


namespace Mountain {

	struct KeyboardEvent : public Event {
		
		KeyboardEvent() = delete;

		virtual ~KeyboardEvent() {}
		
		uint8_t GetKeyCode() const { return m_KeyCode; }

		virtual uint8_t GetCategoryFlags() const override { 
			return EVENT_CATEGORY_INPUT | EVENT_CATEGORY_KEYBOARD;
			
		}


	protected:

		KeyboardEvent(uint8_t KeyCode, Window* win)
			: m_KeyCode(KeyCode), Event(win)
		{

		}

		uint8_t m_KeyCode;
	};
		

	struct KeyPressEvent : public KeyboardEvent {
		
		KeyPressEvent() = delete;
		
		virtual ~KeyPressEvent() {}

		KeyPressEvent(uint8_t KeyCode, uint16_t RepeatCount, Window* win)
			: KeyboardEvent(KeyCode, win), m_RepeatCount(RepeatCount)
		{

		}

		virtual std::string ToString() const override {
			
			return "KeyPressEvent: KeyCode = " + std::to_string(m_KeyCode) + " (RepeatCount =  " + std::to_string(m_RepeatCount) + ")";

		}

		virtual EventTypes GetEventType() const override {
			return EventTypes::KeyPress;
		}

		uint16_t GetRepeatCount() const { return m_RepeatCount; }

	private:
		uint16_t m_RepeatCount;

	
	};

	struct KeyReleaseEvent : public KeyboardEvent {

		KeyReleaseEvent() = delete;

		virtual ~KeyReleaseEvent() {}

		KeyReleaseEvent(uint8_t KeyCode, Window* win)
			: KeyboardEvent(KeyCode, win)
		{

		}

		virtual std::string ToString() const override {

			return "KeyReleaseEvent: KeyCode = " + std::to_string(m_KeyCode);

		}

		virtual EventTypes GetEventType() const override {
			return EventTypes::KeyRelease;
		}


	};


}