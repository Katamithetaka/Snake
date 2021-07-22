#pragma once
#ifndef D9DFC716_5B57_4B6C_9653_5686764F162F
#define D9DFC716_5B57_4B6C_9653_5686764F162F


#define VULKAN_HPP_NO_EXCEPTIONS 
#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC != 1
	#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
	#include <vulkan/vulkan.hpp>
#else
	#include <vulkan/vulkan.hpp>
#endif

#include <set>
#include <vector>
#include <map>
#include <optional>
#include "Logger.hpp"
#include "Result.hpp"


namespace Mountain 
{
	struct SwapChainSupportDetails 
	{
		SwapChainSupportDetails (vk::PhysicalDevice device, vk::SurfaceKHR surface)
		
		{
			vk::Result result;
			std::tie(result, formats) = device.getSurfaceFormatsKHR(surface);


			if(result != vk::Result::eSuccess) 
			{
				foundCapabilities = false;
			}
			else
			{
				foundCapabilities = true;
			}

			std::tie(result, presentModes) = device.getSurfacePresentModesKHR(surface);
			std::tie(result, capabilities) = device.getSurfaceCapabilitiesKHR(surface);

			std::cout << "Found Swapchain capabilities: " << IsComplete() << "\n";
		}

		bool IsComplete() { return foundCapabilities && !formats.empty() && !presentModes.empty();}
		static bool IsSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface) { return SwapChainSupportDetails(device, surface).IsComplete(); }

		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;

		private:
		
		bool foundCapabilities = true;
	};

	struct DefaultQueueIndices 
	{
		DefaultQueueIndices();
		DefaultQueueIndices(vk::PhysicalDevice device, vk::SurfaceKHR surface);
		DefaultQueueIndices(const DefaultQueueIndices&);

		using ArrayType = std::array<uint32_t, 2>;

		std::optional<uint32_t> graphicsQueue, presentQueue;

		ArrayType ToArray() const { return { graphicsQueue.value(), presentQueue.value() };}

		bool IsComplete() const { return graphicsQueue.has_value() && presentQueue.has_value(); }

		static DefaultQueueIndices Get(vk::PhysicalDevice device, vk::SurfaceKHR surface) { return DefaultQueueIndices(device, surface); }
		
		static bool IsSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface) { 
			bool returnVal = DefaultQueueIndices(device, surface).IsComplete(); 
			MTN_INFO("Has queue indices: {0}", returnVal);
			return returnVal;

		}
	};

	SwapChainSupportDetails QuerySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface);

	struct DefaultQueues
	{	
		vk::Queue graphicsQueue, presentQueue;
	};

	struct DefaultDevicePicker
	{

		using QueueIndicesType = DefaultQueueIndices;

		ResultType<Result, vk::PhysicalDevice>::type PickPhysicalDevice(std::vector<vk::PhysicalDevice> devices, vk::SurfaceKHR surface, const std::vector<const char*>&  extensions)
		{
			std::multimap<int, vk::PhysicalDevice> candidates;

			for (const auto& device : devices) {
				int score = ScoreDevice(device, surface, extensions);
				MTN_INFO("Scored: {0}", score);
				candidates.insert(std::make_pair(score, device));
			}

			// Check if the best candidate is suitable at all
			if (candidates.rbegin()->first > 0) {
				return std::make_tuple(Result::eSuccess, candidates.rbegin()->second);
			} else {
				return std::make_tuple(Result::eNoSuitableDevices, vk::PhysicalDevice(VK_NULL_HANDLE));
			}
		}

	private:

		int ScoreDevice(vk::PhysicalDevice device, vk::SurfaceKHR surface, const std::vector<const char*>&  extensions) 
		{
			vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
			MTN_INFO("Got device properties");
			vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();
			MTN_INFO("Got device features");
		
			int score = 0;

			// Discrete GPUs have a significant performance advantage
			if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
				score += 1000;
			}

			// Maximum possible size of textures affects graphics quality
			score += deviceProperties.limits.maxImageDimension2D;

			// Application can't function without geometry shaders or without nescessary extensions
			if (!deviceFeatures.geometryShader || !HasExtensions(device, extensions) || !QueueIndicesType::IsSuitable(device, surface) || !SwapChainSupportDetails::IsSuitable(device, surface)) {
				return 0;
			}

			return score;
		}

		bool HasExtensions(vk::PhysicalDevice device, const std::vector<const char*>& extensions)
		{
			auto [result, extensionProperties] = device.enumerateDeviceExtensionProperties();
			if(result != vk::Result::eSuccess)
			{
				return false;
			}

			std::set<std::string> extensionSet(extensions.begin(), extensions.end());

			for (const auto& extension : extensionProperties) {
				extensionSet.erase(extension.extensionName);
			}

			MTN_INFO("Has extensions: {0}", extensionSet.empty());
			return extensionSet.empty();
		}

	};
}

#endif /* D9DFC716_5B57_4B6C_9653_5686764F162F */
