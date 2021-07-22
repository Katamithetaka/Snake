#include "Device.hpp"
namespace Mountain 
{
	DefaultQueueIndices::DefaultQueueIndices(vk::PhysicalDevice device, vk::SurfaceKHR surface)
	{
		auto families = device.getQueueFamilyProperties();
		MTN_INFO("Found queue family properties");
	
		for(uint32_t i = 0; i < families.size(); ++i)
		{
			if(families[i].queueFlags & vk::QueueFlagBits::eGraphics)
			{
				MTN_INFO("Found graphics queue");
				graphicsQueue = i;	
			} 

			auto [result, j] = device.getSurfaceSupportKHR(i, surface);

			MTN_INFO("Surface support khr: {0}, {1}", vk::to_string(result), j);

			if(result != vk::Result::eSuccess) std::cout << "Couldn't get surface support khr";
			if(result == vk::Result::eSuccess && j) presentQueue = i;

			if(IsComplete()) 
			{
				MTN_INFO("Got queue indices");
				return;
			}
		}
	}

	SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface)
	{
		return SwapChainSupportDetails(device, surface);
	}


}