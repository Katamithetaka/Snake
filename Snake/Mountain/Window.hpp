#pragma once
#ifndef AC9421C9_03F1_47CD_96FB_8EC6D578A252
#define AC9421C9_03F1_47CD_96FB_8EC6D578A252

#define VKFW_NO_EXCEPTIONS

#include <vkfw/vkfw.hpp>
#include <tuple>

namespace Mountain 
{
	std::tuple<size_t, size_t, vkfw::Result> GetFramebufferSize(vkfw::Window window);
}
#endif /* AC9421C9_03F1_47CD_96FB_8EC6D578A252 */
