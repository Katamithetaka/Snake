#pragma once


#ifdef _WIN32
#ifdef UNICODE
#undef UNICODE
#endif
#define VK_USE_PLATFORM_WIN32_KHR 1


#elif defined __linux

// User defined macros cause i probably won't be making code for linux.
#ifdef LINUX_USE_XCB

#define VK_USE_PLATFORM_XCB_KHR

// User defined macros cause i probably won't be making code for linux.
#elif defined LINUX_USE_XLIB

#define VK_USE_PLATFORM_XLIB_KHR

#else 

#error Not supported.

#endif

#else 

#error Not supported.

#endif

#include <vulkan/vulkan.h>
#include "KeyCodes.h"