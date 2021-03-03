#pragma once

#include "Event.h"

namespace Mountain {

	struct MouseEvent : public Event {

		MouseEvent() = delete;
		virtual ~MouseEvent() {}


		int32_t GetXPos() const { return m_XPos;  }
		int32_t GetYPos() const { return m_YPos; }

	protected:
	
		MouseEvent(int32_t xPos, int32_t yPos, Window* win)
			: m_XPos(xPos), m_YPos(yPos), Event(win)
		{

		}


		int32_t m_XPos, m_YPos;

	};

	struct MouseButtonEvent : public MouseEvent {
		
		MouseButtonEvent() = delete;
		virtual ~MouseButtonEvent() {}

		uint8_t GetMouseButton() const { return m_Button; }

	protected:

		MouseButtonEvent(int32_t xPos, int32_t yPos, uint8_t button, Window* win)
			: MouseEvent(xPos, yPos, win), m_Button(button)
		{

		}

		uint8_t m_Button;

	};

	struct MouseScrollEvent : public MouseEvent {
		
		MouseScrollEvent() = delete;

		MouseScrollEvent(int32_t xPos, int32_t yPos, int32_t xOffset, int32_t yOffset, Window* win)
			: MouseEvent(xPos, yPos, win), m_XOffset(xOffset), m_YOffset(yOffset)
		{

		}


		int32_t GetXOffset() const { return m_XOffset; }
		int32_t GetYOffset() const { return m_YOffset; }

		virtual uint8_t GetCategoryFlags() const override {

			return EVENT_CATEGORY_INPUT | EVENT_CATEGORY_MOUSE;

		}

		virtual EventTypes GetEventType() const override {
			return EventTypes::MouseScroll;
		}

		virtual std::string ToString() const override {

			return "MouseScrollEvent : ScrollOffset = {" + std::to_string(m_XOffset) + " , " + std::to_string(m_YOffset) + "}, MousePosition = {" + std::to_string(m_XPos) + " , " + std::to_string(m_YPos) + "}";

		}

	private:

		int32_t m_XOffset, m_YOffset;

	};

	struct MouseMoveEvent : public MouseEvent {

		MouseMoveEvent() = delete;

		MouseMoveEvent(int32_t xPos, int32_t yPos, Window* win)
			: MouseEvent(xPos, yPos, win)
		{

		}


		virtual uint8_t GetCategoryFlags() const override {

			return EVENT_CATEGORY_INPUT | EVENT_CATEGORY_MOUSE;

		}

		virtual EventTypes GetEventType() const override {
			return EventTypes::MouseMove;
		}

		virtual std::string ToString() const override {

			return "MouseMoveEvent:  MousePosition = {" + std::to_string(m_XPos) + " , " + std::to_string(m_YPos) + "}";

		}

	};

	struct MouseButtonPressEvent : public MouseButtonEvent {

		MouseButtonPressEvent() = delete;

		MouseButtonPressEvent(int32_t xPos, int32_t yPos, uint8_t button, Window* win)
			: MouseButtonEvent(xPos, yPos, button, win)
		{

		}

		virtual uint8_t GetCategoryFlags() const override {

			return EVENT_CATEGORY_INPUT | EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_MOUSE_BUTTON;

		}

		virtual EventTypes GetEventType() const override {
			return EventTypes::MouseButtonPress;
		}

		virtual std::string ToString() const override {

			return "MouseButtonPressEvent:  MouseButton = " + std::to_string(m_Button) + ", MousePosition = {" + std::to_string(m_XPos) + " , " + std::to_string(m_YPos) + "}";

		}

	};

	struct MouseButtonReleaseEvent : public MouseButtonEvent {

		MouseButtonReleaseEvent() = delete;

		MouseButtonReleaseEvent(int32_t xPos, int32_t yPos, uint8_t button, Window* win)
			: MouseButtonEvent(xPos, yPos, button, win)
		{

		}

		virtual uint8_t GetCategoryFlags() const override {

			return EVENT_CATEGORY_INPUT | EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_MOUSE_BUTTON;

		}

		virtual EventTypes GetEventType() const override {
			return EventTypes::MouseButtonRelease;
		}

		virtual std::string ToString() const override {

			return "MouseButtonReleaseEvent:  MouseButton = " + std::to_string(m_Button) + ", MousePosition = {" + std::to_string(m_XPos) + " , " + std::to_string(m_YPos) + "}";

		}

	};

}