#include "SnakeApp.hpp"

int main()
{
	try {
		SnakeApp app;	
		app.Run();
	}
	catch(std::exception e) {
		MTN_CRITICAL(e.what());
	}

//	std::cin.get();
	
// double totalTime = vkfw::getTime();
// double lastFrame = totalTime;
// int frames = 0;
// while(!window.shouldClose())
// {
// 	totalTime += vkfw::getTime() - lastFrame;
// 	lastFrame = vkfw::getTime();
// 	++frames;
// 	if(totalTime > 1.0)
// 	{
// 
// 		std::stringstream ss;
// 
// 		ss << "Snake Game - " << frames << " fps";
// 		std::string s = ss.str();
// 		window.setTitle(ss.str().c_str());
// 
// 		totalTime -= 1.0;
// 		frames = 0;
// 	}
// 
// 	vkfw::pollEvents();
// }
// 
// window.destroy();

	return 0;
}