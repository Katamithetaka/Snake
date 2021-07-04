#ifndef MOUNTAIN_APP_INCLUDED
#define MOUNTAIN_APP_INCLUDED

#include <vulkan/vulkan.hpp>
#include <vkfw/vkfw.hpp>
#include <set>
#include <vector>
#include <map>
#include <vkfw/vkfw.hpp>
#include <fstream>
#include <iostream>
#include "Logger.hpp"

namespace Mountain 
{

	struct SwapChainSupportDetails {
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;

		bool IsComplete() 
		{
			return formats.size() && presentModes.size();
		}
	};

	struct Application 
	{
		Application() 
		{
			Logger::Init();
		}
		

		void CreateInstance(const char* AppName, uint32_t AppVersion, uint32_t ApiVersion, const std::vector<const char*>& extensions, const std::vector<const char*>& layers) 
		{
			auto requiredExtensions = GetExtensions(extensions);
			VerifyLayers(layers);
			Layers = layers;

			vk::ApplicationInfo appInfo(AppName, AppVersion, "MountainEngine", VK_MAKE_API_VERSION(0, 1, 0, 0), ApiVersion);
			Instance = vk::createInstance(vk::InstanceCreateInfo(vk::InstanceCreateFlagBits(0), &appInfo, layers, requiredExtensions));

		}

		void CreateDebugMessenger(vk::DebugUtilsMessengerCreateInfoEXT createInfo)
		{
			if(Instance)
			{
				vk::DispatchLoaderDynamic loader(Instance, vkGetInstanceProcAddr);
				DebugMessenger = Instance.createDebugUtilsMessengerEXT(createInfo, nullptr, loader);
			}
			else
			{
				MTN_CORE_ERROR("Tried to create a Debug Messenger without creating an Instance.");
			}
		}


		// Checks for missing extensions automatically to make sure they are compatible with physical devices.
		vk::PhysicalDevice PickPhysicalDevice(std::function<bool(vk::PhysicalDevice)> isDeviceSuitable, std::function<int(vk::PhysicalDevice)> rateDevice, std::vector<const char*> deviceExtensions)
		{
			auto devices = Instance.enumeratePhysicalDevices();

			std::multimap<int, vk::PhysicalDevice> set;
			for(auto& device : devices)
			{
				if(isDeviceSuitable(device) && CheckDeviceExtensions(device, deviceExtensions))
				{
					set.insert(std::make_pair(rateDevice(device), device));
				}
			}

			if(set.empty())
			{
				std::stringstream error;

				error << "None of the " << devices.size() << " devices were suitable.\n";

				throw std::runtime_error(error.str().c_str());
			}

			return PhysicalDevice = set.rbegin()->second;
		}

		vk::SurfaceKHR CreateSurface()
		{
			return Surface = vkfw::createWindowSurface(Instance, Window);
		}

		vk::SurfaceKHR CreateSurface(vkfw::Window window)
		{
			return Surface = vkfw::createWindowSurface(Instance, window);
		}

		vk::Device CreateDevice(vk::PhysicalDeviceFeatures features, const std::vector<vk::DeviceQueueCreateInfo>& queueCreateInfos, const std::vector<const char*>& deviceExtensions) 
		{
			return Device = PhysicalDevice.createDevice(vk::DeviceCreateInfo(vk::DeviceCreateFlagBits(0), queueCreateInfos, Layers, deviceExtensions, &features));
		}

		vk::SwapchainKHR CreateSwapChain(vk::SwapchainCreateInfoKHR createInfo)
		{
			SwapChainImageFormat = createInfo.imageFormat;
			SwapChainExtent = createInfo.imageExtent;

			SwapChain = Device.createSwapchainKHR(createInfo);
			SwapChainImages = Device.getSwapchainImagesKHR(SwapChain);

			return SwapChain;
		}

		SwapChainSupportDetails QuerySwapchainSupport(vk::PhysicalDevice device)
		{
			SwapChainSupportDetails details;
			details.formats = device.getSurfaceFormatsKHR(Surface);
			details.capabilities = device.getSurfaceCapabilitiesKHR(Surface);
			details.presentModes = device.getSurfacePresentModesKHR(Surface);

			return details;
		}



		// Note that the window will be deleted automatically when the app is destroyed.
		vkfw::Window& CreateVKFWWindow(size_t width, size_t height, const char *title, vkfw::WindowHints hints = vkfw::WindowHints{}, vkfw::Monitor monitor = nullptr, vkfw::Window share = nullptr, bool reset_hints = true)
		{
			Window = vkfw::createWindow(width, height, title, hints, monitor, share, reset_hints);
			return Window;
		}

		// Note that the window will be deleted automatically when the app is destroyed.
		vkfw::Window& SetVKFWWindow(vkfw::Window window)
		{
			return Window = window;
		}

		~Application() {
			vk::DispatchLoaderDynamic loader;

			if(Device) loader = vk::DispatchLoaderDynamic(Instance, vkGetInstanceProcAddr, Device, vkGetDeviceProcAddr);
			else if(Instance) loader = vk::DispatchLoaderDynamic(Instance, vkGetInstanceProcAddr);
			
			if(Instance) Instance.destroyDebugUtilsMessengerEXT(DebugMessenger, nullptr, loader);
			if(SwapChain) Device.destroySwapchainKHR(SwapChain);
			if(Surface) Instance.destroySurfaceKHR(Surface);
			if(Device) Device.destroy();
			if(Instance) Instance.destroy();
			if(Window) Window.destroy();
		}

		vk::Device& GetDevice() { return Device; }
		vk::Instance& GetInstance() { return Instance; }
		vk::PhysicalDevice& GetPhysicalDevice() { return PhysicalDevice; }
		vkfw::Window& GetWindow() { return Window; }

	private:

		void VerifyLayers(const std::vector<const char*>& layers)
		{
			auto available = vk::enumerateInstanceLayerProperties();
			std::vector<const char*> unavailable;
			for(auto layer : layers)
			{
				bool found = false;
				for(auto present : available)
				{
					if(strcmp(present.layerName, layer))
					{
						found = true;
						break;
					}
					if(!found)
					{
						unavailable.push_back(layer);
					}
				}
			}

			if(!unavailable.empty())
			{
				std::stringstream error;

				error << "Couldn't find some instance layers: ";

				for(auto layer : unavailable)
				{
					error << layer << " ";
				}

				error << "\n";

				throw std::runtime_error(error.str().c_str());
			}
		}

		std::vector<const char*> GetExtensions(const std::vector<const char*>& extensions)
		{
			// 1. Get vkfw extensions

			std::set<const char*> extensionSet(extensions.begin(), extensions.end());

			uint32_t size;
			auto vkfwExtensions = vkfw::getRequiredInstanceExtensions(&size);

			// 1.1 Add everything to a set

			for(uint32_t i = 0; i < size; ++i)
			{
				extensionSet.insert(vkfwExtensions[i]);
			}

			// 2. Add everything into a vector

			std::vector<const char*> wantedExtensions(extensionSet.begin(), extensionSet.end());

			// 3. Verify available extensions

			auto availableExtensions = vk::enumerateInstanceExtensionProperties();
			
			std::vector<const char*> unavailableExtensions;

			for(auto wantedExtension : wantedExtensions)
			{
				bool found = false;
				for(auto extension : availableExtensions)
				{
					if(strcmp(extension.extensionName, wantedExtension) == 0)
					{
						found = true;
						break;
					}
				}

				if(!found) unavailableExtensions.push_back(wantedExtension);
			}

			// 4. Throw an error if some aren't found.

			if(!unavailableExtensions.empty())
			{
				std::stringstream error;


				error << "Couldn't find some instance extensions: ";

				for(auto extension : unavailableExtensions)
				{
					error << extension << " ";
				}

				error << "\n";
				std::cout << error.str();
				throw std::runtime_error(error.str().c_str());
			}

			return wantedExtensions;
		}

		bool CheckDeviceExtensions(vk::PhysicalDevice device, const std::vector<const char*> extensions)
		{
			auto properties = device.enumerateDeviceExtensionProperties();

			
			for(auto extension : extensions)
			{
				bool found = false;

				for(auto available : properties)
				{
					if(strcmp(available.extensionName, extension))
					{
						found = true;
						break;
					}
				}

				if(!found) return false;
			}

			return true;
		}


protected:

		std::vector<char> ReadShader(std::string fileName)
		{
			std::ifstream file(fileName, std::ios::ate | std::ios::binary);

			if (!file.is_open()) 
			{
				throw std::runtime_error("failed to open file!");
			}

			size_t fileSize = (size_t) file.tellg();
			std::vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);

			file.close();

			return buffer;
		}


protected:
		vk::Instance Instance;
		vk::PhysicalDevice PhysicalDevice;
		vk::Device Device;
		vk::DebugUtilsMessengerEXT  DebugMessenger;
		vkfw::Window Window;
		vk::SurfaceKHR Surface;
		vk::SwapchainKHR SwapChain;
		std::vector<vk::Image> SwapChainImages;
		vk::Format SwapChainImageFormat;
		vk::Extent2D SwapChainExtent;



		// For backwards compatibility
		std::vector<const char*> Layers;

	};

}


#endif