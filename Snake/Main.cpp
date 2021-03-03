#include <Core/Window.h>
#include <cstdio>

#include <Windows.h>

#define uint64_t unsigned long long

using namespace Mountain;


int main() {

	WindowProps props;
	props.Name = "Device Selection Window";
	props.Width = 230;
	props.Height = 159;
	props.WindowCreationFlags = WINDOW_RESIZABLE;
	props.x = 0;
	props.y = 0;
	props.Parent = nullptr;
	
	Window* window = CreatePlatformWindow(props);

	const char* DropDownOptions[3] = {
		"",
		"Device 1",
		"Device 2"
	};
	int a[3] = { 1, 2, 3 };


	Window* DropDown = CreateDropDown("Select Device: ", 20, 20, 140, 20, window, DropDownOptions, 3);

	Window* Button = CreateButton("Finish", 20, 87, 75, 23, window);
	Window* Cancel = CreateButton("Cancel", 110, 87, 75, 23, window);

	while (!window->IsClosed()) {
		
		Event* e = window->Update();

		if (e) {
			switch (e->GetEventType()) {
				case EventTypes::WindowClose:
					window->Close();
			}
		}
	}

	delete window;
	delete DropDown;
	delete Button;
}

