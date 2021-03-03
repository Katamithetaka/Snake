#include <Core/Window.h>
#include <cstdio>

#define uint64_t unsigned long long

using namespace Mountain;


int main() {

	WindowProps props;
	props.Name = "Device Selection Window";
	props.Width = 310;
	props.Height = 150;
	props.WindowCreationFlags = 0;
	GetDesktopResolution(props.x, props.y);
	props.x = props.x / 2 - props.Width / 2;
	props.y = props.y / 2 - props.Height / 2;

	props.Parent = nullptr;
	
	Window* window = CreatePlatformWindow(props);

	const char* DropDownOptions[3] = {
		"",
		"Device 1",
		"Device 2"
	};
	int a[3] = { 1, 2, 3 };


	const unsigned Width = props.Width;
	const unsigned offsetBorder = Width / 10, space = offsetBorder / 2, buttonWidth = (Width - offsetBorder * 2 - space) / 2, ddWidth = Width - offsetBorder * 2;

	Window* DropDown = CreateDropDown("Select Device: ", offsetBorder, 20, ddWidth, 20, window, DropDownOptions, 3);

	Window* Button = CreateButton("Finish", offsetBorder, 80, buttonWidth, 25, window);
	Window* Cancel = CreateButton("Cancel", offsetBorder + buttonWidth + space, 80, buttonWidth, 25, window);

	while (!window->IsClosed()) {
		
		Event* e = window->Update();

		if (e) {
			switch (e->GetEventType()) {
				case EventTypes::WindowClose:
					window->Close();
				case EventTypes::ChildPress:
					
					if (e->GetWindow() == Button) {
						window->Close();
					}
					else if (e->GetWindow() == Cancel) {
						window->Close();
					}


			}
		}
	}

	delete window;
	delete DropDown;
	delete Button;
}

