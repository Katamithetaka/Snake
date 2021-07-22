#pragma once
#ifndef C23637C1_C5BB_47AD_A953_7638D7AE305A
#define C23637C1_C5BB_47AD_A953_7638D7AE305A


#define VULKAN_HPP_NO_EXCEPTIONS 
#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC != 1
	#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
	#include <vulkan/vulkan.hpp>
#else
	#include <vulkan/vulkan.hpp>
#endif

#include "Result.hpp"

// std
#include <vector>
#include <set>


namespace Mountain
{

	void InitVulkan();
}

#endif /* C23637C1_C5BB_47AD_A953_7638D7AE305A */
