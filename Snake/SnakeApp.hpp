#ifndef SNAKE_APP_HPP
#define SNAKE_APP_HPP

#include "Application.hpp"
#include <optional>

constexpr static size_t MAX_FRAMES_IN_FLIGHT = 2;

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) MTN_TRACE(pCallbackData->pMessage);
	else if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) MTN_INFO(pCallbackData->pMessage);
	else if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) MTN_WARN(pCallbackData->pMessage);
	else if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) MTN_ERROR(pCallbackData->pMessage);

	return VK_FALSE;
};

struct QueueIndices 
{
	QueueIndices(vk::PhysicalDevice device, vk::SurfaceKHR surface)
	{
		auto indices = device.getQueueFamilyProperties();

		int i = 0;
		for (const auto& queueFamily : indices) 
		{
			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) 
			{
				graphicsFamily = i;
			}

			if(device.getSurfaceSupportKHR(i, surface)) presentFamily = i;

			if(IsComplete()) return;

			i++;
		}
	}

	QueueIndices() {}

	std::optional<int> graphicsFamily;
	std::optional<int> presentFamily;

	
	static bool Suitable(vk::PhysicalDevice device, vk::SurfaceKHR surface)
	{
		QueueIndices temp(device, surface);

		return temp.IsComplete();
	}

	std::vector<int> ToArray()
	{
		std::set<int> indices = {graphicsFamily.value(), presentFamily.value()};

		return std::vector<int>(indices.begin(), indices.end());
	}

	bool IsComplete() 
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};


struct SnakeApp : public Mountain::Application 
{
	SnakeApp()
	{
		vkfw::init();
		
#ifndef DIST
		CreateInstance("Snake Game", VK_MAKE_API_VERSION(0, 1, 0, 0), VK_API_VERSION_1_2, { VK_EXT_DEBUG_UTILS_EXTENSION_NAME }, { "VK_LAYER_KHRONOS_validation" });
		CreateDebugMessenger(FillDebugMessengerCreateInfo());
#else	
		CreateInstance("Snake Game", VK_MAKE_API_VERSION(0, 1, 0, 0), VK_API_VERSION_1_2, {}, {});
#endif	
		// TODO: Recreate Swapchain -> Make window resizable.
		// CreateVKFWWindow(400, 400, "Snake Game", vkfw::WindowHints(true, true, true, true, true));

		// TODO: Config
		// GetConfig();
		// CreateVKFWWindow(width, height, "Snake Game", vkfw::WindowHints(true, true, true, true));

		CreateVKFWWindow(400, 400, "Snake Game", vkfw::WindowHints(false, true, true, true, true));
		CreateSurface(Window);
		CreateDevice();
		CreateSwapChain();
		CreateImageViews();
		CreateGraphicsPipeline();
		CreateFrameBuffers();
		CreateCommandPools();
		CreateCommandBuffers();
		CreateSyncObjects();
	}

	~SnakeApp()
	{
		vk::DispatchLoaderDynamic loader;

		if(Device) loader = vk::DispatchLoaderDynamic(Instance, vkGetInstanceProcAddr, Device, vkGetDeviceProcAddr);
		else if(Instance) loader = vk::DispatchLoaderDynamic(Instance, vkGetInstanceProcAddr);

		Device.waitIdle();
		Device.destroyCommandPool(CommandPool);
		for(auto& framebuffer: SwapChainFramebuffers)
		{
			Device.destroyFramebuffer(framebuffer);
		}

		for(auto& imageView : SwapChainImageViews)
		{
			Device.destroyImageView(imageView);
		}

		for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			Device.destroySemaphore(imageAvailableSemaphores[i]);
			Device.destroySemaphore(renderFinishedSemaphores[i]);

			Device.destroyFence(inFlightFences[i]);
		}
		Device.destroyPipeline(GraphicsPipeline);
		Device.destroyPipelineLayout(PipelineLayout);
		Device.destroyRenderPass(RenderPass, nullptr, loader);

	}

	bool IsDeviceSuitable(vk::PhysicalDevice physicalDevice)
	{
		auto feature = physicalDevice.getFeatures();
		return QueueIndices::Suitable(physicalDevice, Surface) && feature.geometryShader && QuerySwapchainSupport(physicalDevice).IsComplete();
	}

	int RatePhysicalDevice(vk::PhysicalDevice physicalDevice)
	{
			auto deviceFeatures = physicalDevice.getFeatures();
			auto deviceProperties = physicalDevice.getProperties();

			int score = 0;

			// Discrete GPUs have a significant performance advantage
			if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			{
				score += 1000;
			}

			// Maximum possible size of textures affects graphics quality
			score += deviceProperties.limits.maxImageDimension2D;

			// Application can't function without geometry shaders
			if (!deviceFeatures.geometryShader) 
			{
				return 0;
			}

			return score;
	}

	vk::SurfaceFormatKHR ChooseSwapFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats) 
		{
			if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) 
			{
					return availableFormat;
			}
		}
		return availableFormats[0];
	}

	vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes) 
		{
			if (availablePresentMode == vk::PresentModeKHR::eMailbox) 
			{
				MTN_INFO("Mailbox Mode");
				return availablePresentMode;
			}
		}

		//for (const auto& availablePresentMode : availablePresentModes)
		//{
		//	if (availablePresentMode == vk::PresentModeKHR::eImmediate) 
		//	{
		//		MTN_INFO("Immediate Mode.")
		//		return availablePresentMode;
		//	}
		//}

		MTN_INFO("VSync Mode.");
		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX) 
		{
				return capabilities.currentExtent;
		} 
		else 
		{
			size_t width, height;
			Window.getFramebufferSize(&width, &height);

			vk::Extent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	void CreateDevice()
	{
		
		{
			using namespace std::placeholders;
			PickPhysicalDevice(std::bind(&SnakeApp::IsDeviceSuitable, this, _1), std::bind(&SnakeApp::RatePhysicalDevice, this, _1), DeviceExtensions);
		}

		indices = QueueIndices(PhysicalDevice, Surface);

		auto uniqueIndices = indices.ToArray();

		float queuePriority = 1.0f;

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos; 
		
		for(auto index : uniqueIndices)
		{
			queueCreateInfos.push_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlagBits(0), index, 1, &queuePriority));
		}

		Application::CreateDevice(vk::PhysicalDeviceFeatures(), queueCreateInfos, DeviceExtensions);


		graphicsQueue = Device.getQueue(indices.graphicsFamily.value(), 0);
		presentQueue = Device.getQueue(indices.presentFamily.value(), 0);
	}

	void CreateSwapChain()
	{
		auto swapChainSupport = QuerySwapchainSupport(PhysicalDevice);

		auto Format 	 = ChooseSwapFormat(swapChainSupport.formats);
		auto Extent 	 = ChooseSwapExtent(swapChainSupport.capabilities);
		auto PresentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		auto uniqueIndices = indices.ToArray();
		std::vector<uint32_t> queueFamilyIndices(uniqueIndices.begin(), uniqueIndices.end());

		vk::SwapchainCreateInfoKHR createInfo(vk::SwapchainCreateFlagBitsKHR(0), Surface, imageCount, Format.format, Format.colorSpace, 
											  Extent, 1, vk::ImageUsageFlagBits::eColorAttachment);
	
		if (indices.graphicsFamily != indices.presentFamily) 
		{
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		} else 
		{
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = PresentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		Application::CreateSwapChain(createInfo);
	}
	
	void CreateImageViews()
	{
		SwapChainImageViews.resize(SwapChainImages.size());

		for(size_t i = 0; i < SwapChainImageViews.size(); ++i)
		{
			vk::ImageViewCreateInfo createInfo(vk::ImageViewCreateFlagBits(0), SwapChainImages[i], vk::ImageViewType::e2D, SwapChainImageFormat);
			createInfo.components.r = vk::ComponentSwizzle::eIdentity;
			createInfo.components.g = vk::ComponentSwizzle::eIdentity;
			createInfo.components.b = vk::ComponentSwizzle::eIdentity;
			createInfo.components.a = vk::ComponentSwizzle::eIdentity;
			createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			SwapChainImageViews[i] = Device.createImageView(createInfo);
		}
	}

	void CreateGraphicsPipeline()
	{
		auto vertShaderCode = Application::ReadShader("shaders/Shader.vert.spv");
    	auto fragShaderCode = Application::ReadShader("shaders/Shader.frag.spv");

		vk::ShaderModule vertexShaderModule = Device.createShaderModule(vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlagBits(0), vertShaderCode.size(), (uint32_t*)vertShaderCode.data()));
		vk::ShaderModule fragmentShaderModule = Device.createShaderModule(vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlagBits(0), fragShaderCode.size(), (uint32_t*)fragShaderCode.data()));

		vk::PipelineShaderStageCreateInfo vertexStageInfo(vk::PipelineShaderStageCreateFlagBits(0), vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main");
		vk::PipelineShaderStageCreateInfo fragmentStageInfo(vk::PipelineShaderStageCreateFlagBits(0), vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main");

		vk::PipelineShaderStageCreateInfo shaderStages[] = {vertexStageInfo, fragmentStageInfo};

		vk::PipelineVertexInputStateCreateInfo vertexStateInfo(vk::PipelineVertexInputStateCreateFlagBits(0), 0, nullptr, 0, nullptr);

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo(vk::PipelineInputAssemblyStateCreateFlagBits(0), vk::PrimitiveTopology::eTriangleList, false);

		vk::Viewport viewport(0.0f, 0.0f, SwapChainExtent.width, SwapChainExtent.height, 0.0f, 1.0f);

		vk::Rect2D scissor({0, 0}, SwapChainExtent);

		vk::PipelineViewportStateCreateInfo viewportState(vk::PipelineViewportStateCreateFlagBits(0), 1, &viewport, 1, &scissor);

		vk::PipelineRasterizationStateCreateInfo rasterizer(vk::PipelineRasterizationStateCreateFlagBits(0));
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = vk::PolygonMode::eFill;
		rasterizer.lineWidth = 1.0;
		rasterizer.cullMode = vk::CullModeFlagBits::eBack;
		rasterizer.frontFace = vk::FrontFace::eClockwise;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		vk::PipelineMultisampleStateCreateInfo multisampling(vk::PipelineMultisampleStateCreateFlagBits(0));
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE; 
		multisampling.alphaToOneEnable = VK_FALSE;

		using vk::ColorComponentFlagBits;

		vk::PipelineColorBlendAttachmentState colorBlendAttachment;
		colorBlendAttachment.colorWriteMask = ColorComponentFlagBits::eR | ColorComponentFlagBits::eG | ColorComponentFlagBits::eB | ColorComponentFlagBits::eA;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
		colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero; 
		colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
		colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne; 
		colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

		vk::PipelineColorBlendStateCreateInfo colorBlending(vk::PipelineColorBlendStateCreateFlagBits(0));
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = vk::LogicOp::eCopy;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		PipelineLayout = Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(vk::PipelineLayoutCreateFlagBits(0), 0, nullptr, 0, nullptr));

		CreateRenderPass();

		vk::GraphicsPipelineCreateInfo pipelineInfo(vk::PipelineCreateFlagBits(0));
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexStateInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;
		pipelineInfo.layout = PipelineLayout;
		pipelineInfo.renderPass = RenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		Device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipelineInfo, nullptr, &GraphicsPipeline);
		Device.destroyShaderModule(vertexShaderModule);
		Device.destroyShaderModule(fragmentShaderModule);
	}

	void CreateRenderPass()
	{
		vk::AttachmentDescription colorAttachment;

		colorAttachment.format = SwapChainImageFormat;
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;



		vk::AttachmentReference colorAttachmentRef;
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::SubpassDescription subpass;
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		vk::SubpassDependency dependency;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.srcAccessMask = vk::AccessFlagBits(0);
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

		vk::RenderPassCreateInfo renderPassInfo;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		RenderPass = Device.createRenderPass(renderPassInfo);
	}

	void CreateFrameBuffers()
	{
		SwapChainFramebuffers.resize(SwapChainImageViews.size());

		for(size_t i = 0; i < SwapChainImageViews.size(); ++i)
		{
			std::array<vk::ImageView, 1> attachments = {
				SwapChainImageViews[i]
			};

			vk::FramebufferCreateInfo framebufferInfo(vk::FramebufferCreateFlagBits(0), RenderPass, attachments, SwapChainExtent.width, SwapChainExtent.height, 1);
		
			SwapChainFramebuffers[i] = Device.createFramebuffer(framebufferInfo);
		}
	}

	void CreateCommandPools()
	{
		CommandPool = Device.createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits(0), indices.graphicsFamily.value()));
	}

	void CreateCommandBuffers()
	{
		CommandBuffers.resize(SwapChainFramebuffers.size());
		vk::CommandBufferAllocateInfo allocInfo(CommandPool, vk::CommandBufferLevel::ePrimary, (uint32_t) CommandBuffers.size());

		// vk::DispatchLoaderDynamic loader(Instance, vkGetInstanceProcAddr, Device, vkGetDeviceProcAddr);

		Device.allocateCommandBuffers(&allocInfo, CommandBuffers.data());

		for(size_t i = 0; i < CommandBuffers.size(); ++i)
		{
			CommandBuffers[i].begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits(0), nullptr));

			vk::RenderPassBeginInfo renderPassInfo(RenderPass, SwapChainFramebuffers[i]);
			renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
			renderPassInfo.renderArea.extent = SwapChainExtent;

			std::array<float, 4> clearColorValue({0.0f, 0.0f, 0.0f, 1.0f});
			vk::ClearValue clearColor = vk::ClearColorValue(clearColorValue); 
			
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			CommandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

			CommandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, GraphicsPipeline);

			// TODO: Square/Circle: 
			// CommandBuffers[i].draw(6, 1, 0, 0);
			CommandBuffers[i].draw(6, 1, 0, 0);

			CommandBuffers[i].endRenderPass();

			CommandBuffers[i].end();
		}

	}



	bool Running()
	{
		return !Window.shouldClose();
	}

	void Run()
	{
		static size_t frames = 0;
		static double lastFrame = vkfw::getTime();

		while(Running())
		{
			double now = vkfw::getTime();
			
			if(now - lastFrame > 1)
			{
				Window.setTitle(std::string("Snake Game - ") + std::to_string(frames) + "FPS");
				frames = 0;
				lastFrame = now;
			}
			
			vkfw::pollEvents();
			Draw();
			++frames;
		}

		Device.waitIdle();
	}

	void CreateSyncObjects()
	{
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(SwapChainImages.size(), VK_NULL_HANDLE);

		vk::SemaphoreCreateInfo semaphoreInfo;

		vk::FenceCreateInfo fenceInfo;
		fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

		for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			imageAvailableSemaphores[i] = Device.createSemaphore(semaphoreInfo);
			renderFinishedSemaphores[i] = Device.createSemaphore(semaphoreInfo);
			inFlightFences[i] = Device.createFence(fenceInfo);
		}
	}

	void Draw()
	{
		Device.waitForFences(1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
		uint32_t imageIndex = Device.acquireNextImageKHR(SwapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE);
		
        if (imagesInFlight[imageIndex]) {

            Device.waitForFences(1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

		std::array<vk::Semaphore, 1> waitSemaphores = {imageAvailableSemaphores[currentFrame]};
		std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
		std::array<vk::CommandBuffer, 1> pCommandBuffer = { CommandBuffers[imageIndex] };
		std::array<vk::Semaphore, 1> signalSemaphores = { renderFinishedSemaphores[currentFrame]};


		Device.resetFences(1, &inFlightFences[currentFrame]);

		vk::SubmitInfo submitInfo = vk::SubmitInfo::SubmitInfo(waitSemaphores, waitStages, pCommandBuffer, signalSemaphores);

		graphicsQueue.submit(1, &submitInfo, inFlightFences[currentFrame]);
		vk::PresentInfoKHR presentInfo;

		presentInfo.waitSemaphoreCount = signalSemaphores.size();
        presentInfo.pWaitSemaphores = signalSemaphores.data();

        std::array<vk::SwapchainKHR, 1> swapChains = {SwapChain};
        presentInfo.swapchainCount = swapChains.size();
        presentInfo.pSwapchains = swapChains.data();

        presentInfo.pImageIndices = &imageIndex;

		presentQueue.presentKHR(presentInfo);

		
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}


	vk::DebugUtilsMessengerCreateInfoEXT FillDebugMessengerCreateInfo()
	{
		vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
		
		createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
		createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
		createInfo.pfnUserCallback = DebugCallback;
		createInfo.pUserData = nullptr;

		return createInfo;
	}

private:
	QueueIndices indices;
	const std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	vk::Queue graphicsQueue, presentQueue;
	std::vector<vk::ImageView> SwapChainImageViews;
	vk::PipelineLayout PipelineLayout;
	vk::RenderPass RenderPass;
	vk::Pipeline GraphicsPipeline;
	std::vector<vk::Framebuffer> SwapChainFramebuffers;
	vk::CommandPool CommandPool;
	std::vector<vk::CommandBuffer> CommandBuffers;
	std::vector<vk::Semaphore> imageAvailableSemaphores, renderFinishedSemaphores;
	std::vector<vk::Fence> inFlightFences, imagesInFlight;
	size_t currentFrame = 0;
};

#endif // SNAKE_APP_HPP