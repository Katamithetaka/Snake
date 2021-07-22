#include "Window.hpp"	

namespace Mountain
{

	 std::tuple<size_t, size_t, vkfw::Result> GetFramebufferSize(vkfw::Window window)
	{
		size_t width, height;

		vkfw::Result result = window.getFramebufferSize(&width, &height);

		return std::tuple<size_t, size_t, vkfw::Result>(width, height, result);
	}
}