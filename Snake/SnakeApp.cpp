#include "SnakeApp.hpp"


#include <fstream>
#include <sstream>
#include <random>

vk::DebugUtilsMessengerCreateInfoEXT FillDebugMessengerCreateInfo();

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT _messageSeverity,
													VkDebugUtilsMessageTypeFlagsEXT _messageType,
													const VkDebugUtilsMessengerCallbackDataEXT* _pCallbackData,
													void* pUserData);

#ifndef DIST
	static std::vector<const char*> enabledLayers = { "VK_LAYER_KHRONOS_validation" };
	static std::vector<const char*> debugExtensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
#else
	static std::vector<const char*> enabledLayers = {};
	static std::vector<const char*> debugExtensions = {};
#endif

bool SnakeApp::Initialize()
{
	#define CHECK_AND_RETURN(x) ok = x; if(!ok) return false

	Mountain::Logger::Init();
	errorHandler(Mountain::Exception("Everything is working fine!", Mountain::Result::eSuccess));
	;
	bool ok;
	Mountain::InitVulkan();
	;
	CHECK_AND_RETURN(InitGLFW());
	;
	GetConfig();
	;
	CHECK_AND_RETURN(CreateInstance());
	;
	CHECK_AND_RETURN(CreateWindowHandle());
	;
	InitApp();
	;


	auto result = glfwCreateWindowSurface(instance, (GLFWwindow*)(window), nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface));
	if(result != VK_SUCCESS)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Couldn't create window surface", vk::Result(result)));
		return false;
	}

	;
	CHECK_AND_RETURN(CreateDevice());
	;
	CHECK_AND_RETURN(CreateSwapChain());
	;
	CHECK_AND_RETURN(CreateImageViews());
	;
	CHECK_AND_RETURN(CreateDescriptorSetLayout());
	;
	CHECK_AND_RETURN(CreateGraphicsPipeline());
	;
	CHECK_AND_RETURN(CreateFramebuffers());
	;
	CHECK_AND_RETURN(CreateCommandPool());
	;
	CHECK_AND_RETURN(AllocateCommandBuffers());
	;
	CHECK_AND_RETURN(CreateSyncObjects());
	;
	CHECK_AND_RETURN(CreateStagingBuffer(std::max(config.MaxVerticesSize, config.MaxIndicesSize), stagingBuffer, stagingBufferMemory));
	;
	CHECK_AND_RETURN(CreateVertexBuffer());
	;
	CHECK_AND_RETURN(CreateIndexBuffer());
	;
	CHECK_AND_RETURN(CreateUniformBuffer());
	;
	CHECK_AND_RETURN(CreateDescriptorPool());
	;
	CHECK_AND_RETURN(CreateDescriptorSet());
	;
	FillIndices();
	;
	CHECK_AND_RETURN(SubmitIndices());
	;
	CHECK_AND_RETURN(Run());
	;

	#undef CHECK_AND_RETURN
	;
	return ok;
}

void SnakeApp::DestroyAll()
{
	DestroySwapChain();

	device.destroyDescriptorSetLayout(descriptorSetLayout);
	
	device.destroyBuffer(uniformBuffer);
	device.freeMemory(uniformBufferMemory);
	device.destroyBuffer(stagingBuffer);
	device.freeMemory(stagingBufferMemory);


	device.destroyBuffer(indexBuffer, nullptr);
	device.freeMemory(indexBufferMemory, nullptr);
	device.destroyBuffer(vertexBuffer, nullptr);
	device.freeMemory(vertexBufferMemory, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		device.destroySemaphore(renderFinishedSemaphores[i], nullptr);
		device.destroySemaphore(imageAvailableSemaphores[i], nullptr);
		device.destroyFence(inFlightFences[i], nullptr);
	}

	device.destroyCommandPool(commandPool);
	device.destroyDescriptorPool(descriptorPool);
	if (debugMessenger) instance.destroyDebugUtilsMessengerEXT(debugMessenger);
	instance.destroySurfaceKHR(surface);
	device.destroy();
	instance.destroy();
	
	vkfw::Result result;

	result = window.destroy();
	result = vkfw::terminate();
}

void SnakeApp::DestroySwapChain()
{

	vk::Result result = device.waitIdle();

	for (auto framebuffer : swapChainFramebuffers) 
	{
		device.destroyFramebuffer(framebuffer, nullptr);
	}

	device.freeCommandBuffers(commandPool, commandBuffers);

	device.destroyPipeline(graphicsPipeline);
	device.destroyPipelineLayout(pipelineLayout);
	device.destroyRenderPass(renderPass);


	for(auto& imageView : imageViews) device.destroyImageView(imageView);

	device.destroySwapchainKHR(swapChain);
}

bool SnakeApp::InitGLFW()
{
	vkfw::Result result = vkfw::init();

	Mountain::Exception exception("Couldn't initialize glfw" , result);

	if(result != vkfw::Result::eSuccess)
	{
		if(errorHandler) errorHandler(exception);
		return false;
	}

	return true;
}

void SnakeApp::GetConfig()
{
	std::ifstream ifs("./config.txt", std::ios_base::ate);

	if(!ifs.is_open())
	{
		std::ofstream ofs("./config.txt");
		std::ifstream defaultConfig("./defaultConfig.txt");

		std::string line;

		while(std::getline(defaultConfig, line))
		{
			if(line.substr(0, 5) == "#####") continue;
			ofs << line << "\n";
		}

		return;
	} 

	std::string line;

	while(std::getline(ifs, line))
	{
		if(line[0] == '#') continue;



		// Code nicely taken from SO and adapted to only take a pair.
		// https://stackoverflow.com/questions/5888022/split-string-by-single-spaces

		size_t pos = line.find( ':' );

		if(pos + 1 >= line.size()) continue;

		std::vector<std::string> strs;
		
		std::transform(line.begin(), line.end(), line.begin(), std::tolower);
		
		strs.push_back( line.substr( 0, pos) );
		strs.push_back(line.substr(pos + 1, line.size()));

		Mountain::trim(strs[0]);
		Mountain::trim(strs[1]);

		std::stringstream sstream(strs[1]);

		if(strs[0] == "fullscreen" && !(sstream >> config.fullscreen)) config.fullscreen = false;
		else if(strs[0] == "width" && (!(sstream >> config.width) || config.width < 400)) config.width = 400;
		else if(strs[0] == "height" && (!(sstream >> config.height) || config.height < 400)) config.height = 400; 
		else if(strs[0] == "gridsize" && (!(sstream >> config.gridCount) || config.gridCount > 80 || config.gridCount < 20)) config.gridCount = 20; 

	}
}

void SnakeApp::InitApp()
{
	snake.applePos = GenerateApple();
	snake.head = glm::vec2((float)(config.gridCount >> 1), (float)(config.gridCount >> 1));
	snake.direction = Direction::None;
	snake.input = Direction::None;
	snake.tailSize = snake.defaultSize;
	snake.tail.resize(snake.defaultSize, {snake.head, Direction::None});
	std::fill(vertices.begin(), vertices.end(), Vertex{{-1.f, -1.f}, {0.f, 0.f, 0.f}, 0.0f});
	for(auto& tailPart : snake.tail) tailPart = {snake.head, Direction::None};
	uniform.mvp = glm::ortho(0.0, (double)config.gridCount, 0.0, double(config.gridCount));
	uniform.circleCenter = snake.applePos + 0.5f;
}

bool SnakeApp::CreateInstance()
{
	vk::Result result;
	std::vector<const char*> instanceExtensions = {};

	uint32_t count;
	auto ext = vkfw::getRequiredInstanceExtensions(&count);

	std::vector<const char*> glfwExt(count);
	for(size_t i = 0; i < count; ++i)
	{
		glfwExt[i] = ext[i];
	}

	instanceExtensions.insert(instanceExtensions.begin(), glfwExt.begin(), glfwExt.end());
	instanceExtensions.insert(instanceExtensions.end(), debugExtensions.begin(), debugExtensions.end());



	vk::ApplicationInfo appInfo{};

	appInfo.apiVersion = VK_API_VERSION_1_2;
	appInfo.pApplicationName = "Snake Game";
	appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 2, 0, 0);
	appInfo.pEngineName = "Mountain Engine";
	appInfo.engineVersion = VK_MAKE_API_VERSION(0, 2, 0, 0);

	std::cout << "Did app info\n";

	
	std::tie(result, instance) = vk::createInstance(vk::InstanceCreateInfo(vk::InstanceCreateFlagBits(0), &appInfo, enabledLayers, instanceExtensions));

	Mountain::Exception exception("Couldn't create a vulkan instance" , result);

	if(!instance) ;

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(exception);
		return false;
	}

	std::cout << "Checked result\n";

	VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

	if(enabledLayers.size())
	{
		auto debugInfo = FillDebugMessengerCreateInfo();

		std::tie(result, debugMessenger) = instance.createDebugUtilsMessengerEXT(debugInfo);

		std::cout << vk::to_string(result) << std::endl;

		exception("Couldn't create vulkan debug messenger" , result);

		if(result != vk::Result::eSuccess)
		{
			if(errorHandler) errorHandler(exception);
			return false;
		}
	}

	return true;
}

bool SnakeApp::CreateWindowHandle()
{
	vkfw::Result result;
	if(!vkfw::vulkanSupported()) return false;

	if(config.fullscreen)
	{

		vkfw::Monitor monitor;
		std::tie(result, monitor) = vkfw::getPrimaryMonitor();

		Mountain::Exception exception("Couldn't create a fullscreen window" , result);

		if(result != vkfw::Result::eSuccess)
		{
			if(errorHandler) errorHandler(exception);
			return false;
		}



		std::tie(result, window) = vkfw::createWindow(config.width, config.height, "Snake game", {}, monitor);
		exception("Couldn't create a glfw window" , result);

		if(result != vkfw::Result::eSuccess)
		{
			if(errorHandler) errorHandler(exception);
			return false;
		}

		return true;
	} 
	std::tie(result, window) = vkfw::createWindow(config.width, config.height, "Snake game", {}, nullptr);
	result = static_cast<vkfw::Result>(glfwGetError(nullptr));
	
	Mountain::Exception exception("Couldn't create a glfw window" , result);
	
	if(result != vkfw::Result::eSuccess)
	{
		if(errorHandler) errorHandler(exception);
		return false;
	}

	using namespace std::placeholders;
	window.callbacks()->on_key = std::bind(&SnakeApp::HandleInput, this, _1, _2, _3, _4, _5);
	window.callbacks()->on_framebuffer_resize = [&](vkfw::Window, size_t, size_t)
	{
		framebufferResized = true;
	};

	return true;
}

void SnakeApp::CreateWindowSurface()
{

}

bool SnakeApp::CreateDevice()
{

	vk::Result result;
	Mountain::DefaultDevicePicker devicePicker;
	std::vector<vk::PhysicalDevice> physicalDevices;
	
	;
	std::tie(result, physicalDevices) = instance.enumeratePhysicalDevices();
	
	Mountain::Exception exception("Couldn't enumerate (or find) any graphic card.", result);

	;

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(exception);
		return false;
	}

	Mountain::Result mtnResult;

	std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	if(!surface) ;

	std::tie(mtnResult, physicalDevice) = devicePicker.PickPhysicalDevice(physicalDevices, surface, deviceExtensions);

	exception("Couldn't find a suitable device", mtnResult);

	if(physicalDevice) ;

	if(mtnResult != Mountain::Result::eSuccess)
	{
		if(errorHandler) errorHandler(exception);
		return false;
	}


	const Mountain::DefaultQueueIndices queueFamilies(physicalDevice, surface);
	auto indices = queueFamilies.ToArray();
	std::set<uint32_t> indicesSet(indices.begin(), indices.end());
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : indicesSet) {
		vk::DeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	vk::PhysicalDeviceFeatures features = physicalDevice.getFeatures();
	vk::DeviceCreateInfo createInfo(vk::DeviceCreateFlagBits(0), queueCreateInfos, enabledLayers, deviceExtensions, &features);


	std::tie(result, device) = physicalDevice.createDevice(createInfo);
	

	exception("Couldn't create logical device", result);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(exception);
		return false;
	}

	VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
	

	queues.graphicsQueue = device.getQueue(queueFamilies.graphicsQueue.value(), 0);
	queues.presentQueue = device.getQueue(queueFamilies.presentQueue.value(), 0);

	return true;
}

bool SnakeApp::CreateSwapChain()
{
	auto swapchainDetails = Mountain::QuerySwapChainSupport(physicalDevice, surface);

 	ChooseSurfaceFormat(swapchainDetails.formats);
	ChooseSwapChainPresentMode(swapchainDetails.presentModes);
	ChooseSwapChainExtent(swapchainDetails.capabilities);

	uint32_t imageCount = swapchainDetails.capabilities.minImageCount + 1;

	if (swapchainDetails.capabilities.maxImageCount > 0 && imageCount > swapchainDetails.capabilities.maxImageCount) 
	{
		imageCount = swapchainDetails.capabilities.maxImageCount;
	}

	vk::Result result;

	vk::SwapchainCreateInfoKHR createInfo{};
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = swapChainExtent;
	createInfo.imageArrayLayers = 1; // always one except if stereoscopic 3D application
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

	const Mountain::DefaultQueueIndices queueFamilies(physicalDevice, surface);
	auto indices = queueFamilies.ToArray();
	auto indicesSet = std::set(indices.begin(), indices.end());
	// There will be only 1 value if all indices are the same.
	if(indicesSet.size() != 1 && indices.size()) 
	{
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = indices.size();
		createInfo.pQueueFamilyIndices = indices.data();
	} 
	else
	{
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr; 
	}

	createInfo.preTransform = swapchainDetails.capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = swapChainPresentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	std::tie(result, swapChain) = device.createSwapchainKHR(createInfo);

	Mountain::Exception exception("Couldn't create swapchain", result);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(exception);
		return false;
	}

	std::tie(result, images) = device.getSwapchainImagesKHR(swapChain);


	exception("Couldn't retrieve swapchain images", result);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(exception);
		return false;
	}

	swapChainImageFormat = surfaceFormat.format;

	return true;
}

bool SnakeApp::CreateImageViews()
{
	vk::Result result = vk::Result::eErrorUnknown;
	Mountain::Exception exception("", result);

	imageViews.resize(images.size());
	for (size_t i = 0; i < images.size(); i++) 
	{
		vk::ImageViewCreateInfo createInfo{};
		createInfo.image = images[i];
		createInfo.viewType = vk::ImageViewType::e2D;
		createInfo.format = swapChainImageFormat;
		createInfo.components.r = vk::ComponentSwizzle::eIdentity;
		createInfo.components.g = vk::ComponentSwizzle::eIdentity;
		createInfo.components.b = vk::ComponentSwizzle::eIdentity;
		createInfo.components.a = vk::ComponentSwizzle::eIdentity;
		createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1; // Only would create multiple layers with a stereoscopic 3D application.

		std::tie(result, imageViews[i]) = device.createImageView(createInfo);

		if(result != vk::Result::eSuccess)
		{
			exception("Couldn't create a swapchain image view", result);
			if(errorHandler) errorHandler(exception);
			return false;
		}
	}

	return true;
}

bool SnakeApp::CreateDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics;

	vk::DescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	vk::Result result;
	std::tie(result, descriptorSetLayout) = device.createDescriptorSetLayout(layoutInfo);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Couldn't create descriptor set layout for ubo", result));
		return false;
	}
	return true;
}

bool SnakeApp::CreateGraphicsPipeline()
{

	vk::Result result;
	vk::ShaderModule vertShaderModule;
	std::tie(result, vertShaderModule) = CreateShaderModule("./Shaders/Shader.vert.spv");

	Mountain::Exception exception("Couldn't create vertex shader", result);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(exception);
		return false;
	}



	vk::ShaderModule fragShaderModule;
	std::tie(result, fragShaderModule) = CreateShaderModule("./Shaders/Shader.frag.spv");

	exception("Couldn't create fragment shader", result);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(exception);
		return false;
	}
	
	std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;
	vk::PipelineShaderStageCreateInfo& vertexShaderStage = shaderStages[0], &fragmentShaderStage = shaderStages[1];

	vertexShaderStage.pName = "main";
	vertexShaderStage.stage = vk::ShaderStageFlagBits::eVertex;
	vertexShaderStage.module = vertShaderModule;

	fragmentShaderStage.pName = "main";
	fragmentShaderStage.stage = vk::ShaderStageFlagBits::eFragment;
	fragmentShaderStage.module = fragShaderModule;

	vk::Viewport viewport = GetViewport();
	uniform.SetViewport(viewport);
	vk::Rect2D scissor = GetScissor();

	vk::PipelineViewportStateCreateInfo viewportState{};
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	auto bindingDescription = Vertex::GetBindingDescription();
	auto attributeDescriptions = Vertex::GetAttributeDescriptions();

	vk::PipelineVertexInputStateCreateInfo vertexInput{};
	vertexInput.vertexBindingDescriptionCount = 1;
	vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInput.pVertexBindingDescriptions = &bindingDescription;
	vertexInput.pVertexAttributeDescriptions = attributeDescriptions.data();

	auto inputAssembly = GetInputAssemblyInfo();
	auto rasterizer = GetRasterizationInfo();
	auto multisampling = GetMultisampleingInfo();
	auto colorBlendAttachment = GetColorBlendAttachment();
	auto colorBlending = GetColorBlendInfo(colorBlendAttachment);

	if(!CreatePipelineLayout()) return false;
	if(!CreateRenderPass()) return false;

	vk::GraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInput;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; 
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; 
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; 
	pipelineInfo.basePipelineIndex = -1; 

	std::tie(result, graphicsPipeline) = device.createGraphicsPipeline(nullptr, pipelineInfo);

	exception("Couldn't create graphics pipeline", result);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(exception);
		return false;
	}

	device.destroyShaderModule(vertShaderModule);
	device.destroyShaderModule(fragShaderModule);

	return true;
}

bool SnakeApp::CreateFramebuffers()
{
	swapChainFramebuffers.resize(imageViews.size());
	vk::Result result;

	for (size_t i = 0; i < imageViews.size(); i++) {
		vk::ImageView attachments[] = {
			imageViews[i]
		};

		vk::FramebufferCreateInfo framebufferInfo{};
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		std::tie(result, swapChainFramebuffers[i]) = device.createFramebuffer(framebufferInfo);

		if(result != vk::Result::eSuccess)
		{
			Mountain::Exception exception("Couldn't create framebuffer", result);
			if(errorHandler) errorHandler(exception);
			return false;
		}
	}

	return true;
}

bool SnakeApp::CreateCommandPool()
{
	const Mountain::DefaultQueueIndices queueFamilies(physicalDevice, surface);
	
	vk::CommandPoolCreateInfo poolInfo{};
	poolInfo.queueFamilyIndex = queueFamilies.graphicsQueue.value();
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

	vk::Result result;

	std::tie(result, commandPool) = device.createCommandPool(poolInfo);

	if(result != vk::Result::eSuccess)
	{
		Mountain::Exception exception("Couldn't create command pool", result);
		if(errorHandler) errorHandler(exception);
		return false;
	}

	return true;
}

bool SnakeApp::AllocateCommandBuffers()
{
	vk::Result result;

	commandBuffers.resize(swapChainFramebuffers.size());

	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.commandPool = commandPool;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();	
	std::tie(result, commandBuffers) = device.allocateCommandBuffers(allocInfo);

	
	if(result != vk::Result::eSuccess)
	{
		Mountain::Exception exception("Couldn't allocate command buffers", result);
		if(errorHandler) errorHandler(exception);
		return false;
	}

	return true;
}

bool SnakeApp::RecreateCommandBuffer()
{
	vk::Result result;
	const auto& commandBuffer = commandBuffers[currentFrame];
	
	vk::CommandBufferBeginInfo beginInfo{};

	result = commandBuffer.begin(beginInfo);

	if(result != vk::Result::eSuccess)
	{
		Mountain::Exception exception("Couldn't begin command buffer", result);
		if(errorHandler) errorHandler(exception);
		return false;
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapChainFramebuffers[currentFrame];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = swapChainExtent;

	VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	
		
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

		vk::Buffer vertexBuffers[] = {vertexBuffer};
		vk::DeviceSize offsets[] = {0};

		commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

		commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

		commandBuffer.drawIndexed(static_cast<uint32_t>((2 + snake.tail.size()) * 6), 1, 0, 0, 0);

	commandBuffer.endRenderPass();

	result = commandBuffer.end();

	if(result != vk::Result::eSuccess)
	{
		Mountain::Exception exception("Couldn't end command buffer", result);
		if(errorHandler) errorHandler(exception);
		return false;
	}

	return true;
}

bool SnakeApp::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory)
{
	vk::Result result;
	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	std::tie(result, buffer) = device.createBuffer(bufferInfo);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Couldn't create buffer", result));
		return false;
	}

	vk::MemoryRequirements memRequirements;
	memRequirements = device.getBufferMemoryRequirements(buffer);

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.allocationSize = memRequirements.size;

	auto [found, index] = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if(!found)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Couldn't find an appropriate memory type for buffer"));
		return false;
	}

	allocInfo.memoryTypeIndex = index;

	std::tie(result, bufferMemory) = device.allocateMemory(allocInfo);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Couldn't allocate memory for buffer", result));
		return false;
	}

	result = device.bindBufferMemory(buffer, bufferMemory, 0);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Couldn't bind memory to buffer", result));
		return false;
	}

	return true;
}

bool SnakeApp::CreateStagingBuffer(size_t size, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory)
{
	return CreateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible, buffer, bufferMemory);
}

std::tuple<bool, uint32_t> SnakeApp::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
	vk::Result result;	
	vk::PhysicalDeviceMemoryProperties memProperties;
	memProperties = physicalDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return std::make_tuple(true, i);
		}
	}

	return std::make_tuple(false, 0);
}

bool SnakeApp::CreateVertexBuffer()
{
	return CreateBuffer(config.MaxVerticesSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer, vertexBufferMemory);
}

bool SnakeApp::CreateIndexBuffer()
{
	return CreateBuffer(config.MaxIndicesSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer, indexBufferMemory);
}

bool SnakeApp::CreateDescriptorPool()
{
	vk::DescriptorPoolSize poolSize;
	poolSize.descriptorCount = static_cast<uint32_t>(1);

	vk::DescriptorPoolCreateInfo poolInfo{};
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 2;


	vk::Result result;
	std::tie(result, descriptorPool) = device.createDescriptorPool(poolInfo);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Couldn't create descriptor pool", result));
		return false;
	}

	return true;
}

bool SnakeApp::CreateDescriptorSet()
{
	vk::DescriptorSetAllocateInfo allocInfo{};
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	auto [result, descriptorSets] = device.allocateDescriptorSets(allocInfo);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Couldn't create descriptor set", result));
		return false;
	}

	descriptorSet = descriptorSets[0];

	vk::DescriptorBufferInfo bufferInfo;
	bufferInfo.buffer = uniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(Uniform);

	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;

	device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
	

	return true;
}

bool SnakeApp::CreateUniformBuffer()
{
	return CreateBuffer(sizeof(Uniform), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible, uniformBuffer, uniformBufferMemory);
}

bool SnakeApp::CopyBuffer(size_t size, vk::Buffer& src, vk::Buffer& dst)
{

	vk::Result result;
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	vk::CommandBuffer commandBuffer;
	result = device.allocateCommandBuffers(&allocInfo, &commandBuffer);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Couldn't allocate command buffer to copy buffers", result));
		return false;
	}

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	result = commandBuffer.begin(beginInfo);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Couldn't begin recording command buffer to copy buffers", result));
		return false;
	}

	vk::BufferCopy copyRegion{};
	copyRegion.dstOffset = 0;
	copyRegion.srcOffset = 0;
	copyRegion.size = size;

	commandBuffer.copyBuffer(src, dst, 1, &copyRegion);

	result = commandBuffer.end();

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Couldn't end recording command buffer to copy buffers", result));
		return false;
	}

	vk::SubmitInfo submitInfo{};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	result = queues.graphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Couldn't submit command to copy buffers to graphics queue", result));
		return false;
	}

	result = queues.graphicsQueue.waitIdle();

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Couldn't wait for graphics queue to be idle after copying buffers", result));
		return false;
	}

	device.freeCommandBuffers(commandPool, 1, &commandBuffer);


	return true;
}

void SnakeApp::FillVertices()
{

	vertices[0] = {glm::vec2(snake.head) + glm::vec2(0.0, 0.0), snake.headColor, 0}; // bottom left
	vertices[1] = {glm::vec2(snake.head) + glm::vec2(0.0, 1.0), snake.headColor, 0}; // top left
	vertices[2] = {glm::vec2(snake.head) + glm::vec2(1.0, 1.0), snake.headColor, 0}; // top right
	vertices[3] = {glm::vec2(snake.head) + glm::vec2(1.0, 0.0), snake.headColor, 0}; // bottom right
	
	for(size_t i = 0; i < snake.tail.size(); ++i)
	{
		size_t j = (i + 1) * 4;
		vertices[j + 0] = {glm::vec2(snake.tail[i].position) + glm::vec2(0.0, 0.0), snake.color, 0}; // bottom left
		vertices[j + 1] = {glm::vec2(snake.tail[i].position) + glm::vec2(0.0, 1.0), snake.color, 0}; // top left
		vertices[j + 2] = {glm::vec2(snake.tail[i].position) + glm::vec2(1.0, 1.0), snake.color, 0}; // top right
		vertices[j + 3] = {glm::vec2(snake.tail[i].position) + glm::vec2(1.0, 0.0), snake.color, 0}; // bottom right
	}
	
	size_t index = (snake.tail.size() + 1) * 4;
	
	vertices[index + 0] = {snake.applePos + glm::vec2(0.0, 0.0), snake.appleColor, 1}; // bottom left
	vertices[index + 1] = {snake.applePos + glm::vec2(0.0, 1.0), snake.appleColor, 1}; // top left
	vertices[index + 2] = {snake.applePos + glm::vec2(1.0, 1.0), snake.appleColor, 1}; // top right
	vertices[index + 3] = {snake.applePos + glm::vec2(1.0, 0.0), snake.appleColor, 1}; // bottom right

}

void SnakeApp::FillIndices()
{
	for(size_t i = 0; i < indices.size(); i += 6)
	{
		size_t j = (i/6) * 4;
		indices[i + 0] = j + 0;
		indices[i + 1] = j + 1;
		indices[i + 2] = j + 2;
		indices[i + 3] = j + 2;
		indices[i + 4] = j + 3;
		indices[i + 5] = j + 0;
	}
}

bool SnakeApp::SubmitIndices()
{

    void* data;
	vk::Result result = device.mapMemory(stagingBufferMemory, 0, config.MaxIndicesSize, vk::MemoryMapFlagBits(0), &data);


	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Coudln't map staging buffer memory to fill it with indices", result));
		return false;
	}

	memcpy(data, indices.data(), (size_t) config.MaxIndicesSize);
	device.unmapMemory(stagingBufferMemory);

	CopyBuffer(config.MaxIndicesSize, stagingBuffer, indexBuffer);

	return true;
}

bool SnakeApp::SubmitUniform()
{
	size_t bufferSize = sizeof(Uniform);
	void* data;
	vk::Result result = device.mapMemory(uniformBufferMemory, 0, bufferSize, vk::MemoryMapFlagBits(0), &data);


	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Coudln't map uniform buffer memory", result));
		return false;
	}

	memcpy(data, &uniform, (size_t) bufferSize);
	device.unmapMemory(uniformBufferMemory);

	return true;
}

bool SnakeApp::SubmitVertices()
{
	size_t bufferSize = vertices.size() * sizeof(Vertex);
    void* data;
	vk::Result result = device.mapMemory(stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlagBits(0), &data);


	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Coudln't map staging buffer memory to fill it with vertices", result));
		return false;
	}

	memcpy(data, vertices.data(), (size_t) bufferSize);
	device.unmapMemory(stagingBufferMemory);

	CopyBuffer(bufferSize, stagingBuffer, vertexBuffer);
	
	return true;
}

bool SnakeApp::CreateSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(imageViews.size(), VK_NULL_HANDLE);

	vk::SemaphoreCreateInfo semaphoreInfo{};

	vk::FenceCreateInfo fenceInfo{};
	fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

	vk::Result result;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		std::tie(result, renderFinishedSemaphores[i]) = device.createSemaphore(semaphoreInfo);

		if(result != vk::Result::eSuccess)
		{
			Mountain::Exception exception("Couldn't create render Finished semaphore", result);
			if(errorHandler) errorHandler(exception);
			return false;
		}

		std::tie(result, imageAvailableSemaphores[i]) = device.createSemaphore(semaphoreInfo);

		if(result != vk::Result::eSuccess)
		{
			Mountain::Exception exception("Couldn't create image available semaphore", result);
			if(errorHandler) errorHandler(exception);
			return false;
		}

		std::tie(result, inFlightFences[i]) = device.createFence(fenceInfo);
		
		if(result != vk::Result::eSuccess)
		{
			Mountain::Exception exception("Couldn't create in flight fence", result);
			if(errorHandler) errorHandler(exception);
			return false;
		}
	}

	return true;
}

bool SnakeApp::Run()
{
	double lastFrame = vkfw::getTime().value, totalTime = 0.0;
	int frames = 0;

	while(!window.shouldClose().value)
	{

		{
			vkfw::Result result = vkfw::pollEvents();

		}
		double now = vkfw::getTime().value;

		double deltaTime = now - lastFrame;

		totalTime += deltaTime;
		++frames;
		if(!Update(deltaTime)) 
		{
			vkfw::Result result = window.setShouldClose(true);
			return false;
		}


		if(totalTime >= 1.0)
		{
			totalTime -= 1.0;
			if(!config.fullscreen) vkfw::Result result = window.setTitle("Snake Game - " + std::to_string(frames) + "fps");
			frames = 0;
		}

		lastFrame = now;

		if(!Draw())
		{
			vkfw::Result result = window.setShouldClose(true);
			return false;
		}
	}

	return true;
}

bool SnakeApp::Update(const double& deltaTime)
{
	static double total = 0.0;
	static auto updatePos = [&](glm::vec2& position, Direction dir)
	{
		switch(dir)
		{
			case Direction::Left:    position -=    glm::vec2(1.0, 0.0) * float(snake.speed * deltaTime); break;
			case Direction::Right:   position +=   glm::vec2(1.0, 0.0) * float(snake.speed * deltaTime); break;
			case Direction::Down:    position += glm::vec2(0.0, 1.0) * float(snake.speed * deltaTime); break;
			case Direction::Up:      position -=     glm::vec2(0.0, 1.0) * float(snake.speed * deltaTime); break;
			default: break;
		}
	};

	MTN_INFO(snake.input != Direction::None); 

	if(snake.direction != Direction::None)
	{
		
		total += deltaTime;

		updatePos(snake.head, snake.direction);
		for(size_t i = 0; i < snake.tail.size(); ++i) updatePos(snake.tail[i].position, snake.tail[i].direction);



		if(total >= 1 / snake.speed)
		{
			total = 0;
			if(snake.direction == Direction::Left || snake.direction == Direction::Up) snake.head = glm::ceil(snake.head);
			else snake.head = glm::floor(snake.head);

			for(auto& tailPart : snake.tail)
			{
				if(tailPart.direction == Direction::Left || tailPart.direction == Direction::Up) tailPart.position = glm::ceil(tailPart.position);
				else tailPart.position = glm::floor(tailPart.position);

				if(snake.head == tailPart.position) 
				{
					Reset();
					return true;
				}

	
			}

			for(size_t i = snake.tail.size() - 1; i > 0; --i)
			{
				snake.tail[i].direction = snake.tail[i-1].direction;
			}

			if(snake.tail.size() < snake.tailSize) snake.tail.push_back({snake.tail.back().position, Direction::None});


			snake.tail[0].direction = snake.direction;

			if(snake.head.x == 0 || snake.head.x == config.gridCount || snake.head.y == 0 || snake.head.y == config.gridCount) 
			{	
				Reset();
				return true;
			}

			if(snake.head == snake.applePos)
			{
				snake.tailSize += snake.increment;
				snake.applePos = GenerateApple();
				// Score
			}

			if(snake.input != Direction::None)
			{
				snake.direction = snake.input;
				snake.input = Direction::None;
			}
		}
	}
	if(snake.direction == Direction::None && snake.input != Direction::None) 
	{
		snake.direction = snake.input;
		snake.input = Direction::None;
	}

	FillVertices();
	if(!SubmitVertices()) return false;

	uniform.circleCenter = snake.applePos + glm::vec2(0.5, 0.5);

	;

	if(!SubmitUniform()) return false;

	return true;
}

void SnakeApp::Reset()
{
	InitApp();
}

void SnakeApp::HandleInput(vkfw::Window window, vkfw::Key key, int32_t scancode, vkfw::KeyAction action, vkfw::ModifierKeyFlags flags)
{
	if(action != vkfw::KeyAction::ePress) return;

	if(key == vkfw::Key::eLeft || key == vkfw::Key::eQ || key == vkfw::Key::eA) 
	{
		snake.input = Direction::Left;
	}
	if(key == vkfw::Key::eRight || key == vkfw::Key::eD) 
	{
		snake.input = Direction::Right;
	}
	if(key == vkfw::Key::eUp || key == vkfw::Key::eZ || key == vkfw::Key::eW || key == vkfw::Key::eY) 
	{
		snake.input = Direction::Up;
	}
	if(key == vkfw::Key::eDown || key == vkfw::Key::eS)
	{
		snake.input = Direction::Down;
	}
	if(key == vkfw::Key::eEscape) vkfw::Result result = window.setShouldClose(true);
	
}

bool SnakeApp::RecreateSwapChain()
{
	#define CHECK_AND_RETURN(x) ok = x; if(!ok) return false
	bool ok = false;
	size_t width, height;
	vkfw::Result result;
	result = window.getFramebufferSize(&width, &height);
	size_t attempts = 0;
	while(result != vkfw::Result::eSuccess && attempts < 500)
	{
		result = window.getFramebufferSize(&width, &height);
		vkfw::Result temp = vkfw::waitEvents();
		++attempts;
	}

	vk::Result vkResult;
	vkResult = device.waitIdle();

	if(vkResult != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Couldn't wait for device to be idle", vkResult));
	}

	DestroySwapChain();

	CHECK_AND_RETURN(CreateSwapChain());
	CHECK_AND_RETURN(CreateImageViews());
	CHECK_AND_RETURN(CreateRenderPass());
	CHECK_AND_RETURN(CreateGraphicsPipeline());
	CHECK_AND_RETURN(CreateFramebuffers());
	CHECK_AND_RETURN(AllocateCommandBuffers());
	CHECK_AND_RETURN(RecreateCommandBuffer());

	imagesInFlight.resize(images.size(), VK_NULL_HANDLE);

	return true;
}


bool SnakeApp::Draw()
{
	vk::Result result;
	result = device.waitForFences(1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	if(result != vk::Result::eSuccess) 
	{
		Mountain::Exception exception("Couldn't wait for an In flight fence", result);
		if(errorHandler) errorHandler(exception);
		return false;
	}

	uint32_t imageIndex;

	std::tie(result, imageIndex) = device.acquireNextImageKHR(swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE);

	if (result == vk::Result::eErrorOutOfDateKHR) {
		RecreateSwapChain();
		return true;
	}
	else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) 
	{
		if (errorHandler) errorHandler(Mountain::Exception("Couldn't acquire next swapchain image", result));
		return false;
	}


	imagesInFlight[imageIndex] = inFlightFences[currentFrame];

	vk::SubmitInfo submitInfo;

	if(!RecreateCommandBuffer()) return false;
	
	vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
	vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	result = device.resetFences(1, &inFlightFences[currentFrame]);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(Mountain::Exception("Couldn't reset fences", result));

		return false;
	}

	result = queues.graphicsQueue.submit(1, &submitInfo, inFlightFences[currentFrame]);

	if(result != vk::Result::eSuccess)
	{
		Mountain::Exception exception("Couldn't submit draw command buffer.", result);
		if(errorHandler) errorHandler(exception);
		return true;
	}

	vk::PresentInfoKHR presentInfo{};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	vk::SwapchainKHR swapChains[] = {swapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	result = queues.presentQueue.presentKHR(&presentInfo);

	if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) 
	{
		framebufferResized = false;
		if(!RecreateSwapChain()) return false;
		return true;
	} 
	else if (result != vk::Result::eSuccess) 
	{
		Mountain::Exception exception("Failed to present swap chain image", result);
		if(errorHandler) errorHandler(exception);
		return false;
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	return true;
}


std::vector<char> SnakeApp::ReadShader(const std::string& fileName)
{
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open()) 
	{
		return { '\0' };
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);
	
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

vk::ResultValue<vk::ShaderModule> SnakeApp::CreateShaderModule(const std::string& fileName)
{
	return CreateShaderModule(ReadShader(fileName));
}

vk::ResultValue<vk::ShaderModule> SnakeApp::CreateShaderModule(const std::vector<char>& code)
{
	return device.createShaderModule(vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlagBits(0), code.size(), (const uint32_t*)code.data()));
}

vk::Viewport SnakeApp::GetViewport()
{
	vk::Viewport viewport{};

	const auto size = std::min(swapChainExtent.width, swapChainExtent.height);
	viewport.x = (swapChainExtent.width - size)  >> 1; 
	viewport.y = (swapChainExtent.height - size) >> 1;
	viewport.width = (float)  ((swapChainExtent.width + size)  >> 1); // width - (width - size) / 2 = width / 2 + size / 2 = (width + size) / 2 = (width + size) >> 1
	viewport.height = (float) ((swapChainExtent.height + size) >> 1);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	return viewport;
}

vk::Rect2D SnakeApp::GetScissor()
{
	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = swapChainExtent;

	return scissor;
}


vk::PipelineInputAssemblyStateCreateInfo SnakeApp::GetInputAssemblyInfo()
{
	vk::PipelineInputAssemblyStateCreateInfo createInfo{};
	createInfo.topology = vk::PrimitiveTopology::eTriangleList;
	createInfo.primitiveRestartEnable = VK_FALSE;

	return createInfo;
}

vk::PipelineRasterizationStateCreateInfo SnakeApp::GetRasterizationInfo()
{
	vk::PipelineRasterizationStateCreateInfo createInfo;
	createInfo.depthClampEnable = VK_FALSE;
	createInfo.rasterizerDiscardEnable = VK_FALSE;
	createInfo.polygonMode = vk::PolygonMode::eFill;
	createInfo.cullMode = vk::CullModeFlagBits::eBack;
	createInfo.frontFace = vk::FrontFace::eCounterClockwise;
	createInfo.lineWidth = 1.0f;
	createInfo.depthBiasEnable = VK_FALSE;
	createInfo.depthBiasConstantFactor = 0.0f; 
	createInfo.depthBiasClamp = 0.0f; 
	createInfo.depthBiasSlopeFactor = 0.0f; 

	return createInfo;
}

vk::PipelineMultisampleStateCreateInfo SnakeApp::GetMultisampleingInfo()
{
	vk::PipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
	multisampling.minSampleShading = 1.0f; 
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;
	
	return multisampling;
}

vk::PipelineColorBlendAttachmentState SnakeApp::GetColorBlendAttachment()
{
	// For once i decided to use the C structure as it uses a lot of enums and would require way more writing to access them with the C++ bindings.
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; 
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; 
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; 
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; 
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; 
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; 

	return colorBlendAttachment;
}

vk::PipelineColorBlendStateCreateInfo SnakeApp::GetColorBlendInfo(vk::PipelineColorBlendAttachmentState& colorBlendAttachment) 
{
	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = vk::LogicOp::eCopy; 
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; 
	colorBlending.blendConstants[1] = 0.0f; 
	colorBlending.blendConstants[2] = 0.0f; 
	colorBlending.blendConstants[3] = 0.0f; 

	return colorBlending;
}


bool SnakeApp::CreatePipelineLayout()
{
	vk::Result result;
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

	std::tie(result, pipelineLayout) = device.createPipelineLayout(pipelineLayoutInfo);

	Mountain::Exception exception("Couldn't create pipeline layout", result);
	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(exception);
		return false;
	}

	return true;
}

bool SnakeApp::CreateRenderPass()
{
	vk::AttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
	
	vk::SubpassDescription subpass{};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	vk::Result result;
	std::tie(result, renderPass) = device.createRenderPass(renderPassInfo);

	Mountain::Exception exception("Couldn't create render pass", result);

	if(result != vk::Result::eSuccess)
	{
		if(errorHandler) errorHandler(exception);
		return false;
	}

	return true;
}


void SnakeApp::ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
	for(const auto& availableFormat : availableFormats) 
	{
		if(availableFormat.format == vk::Format::eR8G8B8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) 
		{
			surfaceFormat = availableFormat;
			return;
		}
	}

	surfaceFormat = availableFormats[0];
}

void SnakeApp::ChooseSwapChainPresentMode(const std::vector<vk::PresentModeKHR> & availablePresentModes)
{
	for(const auto& availablePresentMode : availablePresentModes) 
	{
		if(availablePresentMode == vk::PresentModeKHR::eMailbox) 
		{
			MTN_INFO("MailBox Mode");
			swapChainPresentMode = availablePresentMode;
			return;
		}
	}

	for(const auto& availablePresentMode : availablePresentModes) 
	{
		if(availablePresentMode == vk::PresentModeKHR::eImmediate) 
		{
			MTN_INFO("Immediate Mode");
			swapChainPresentMode = availablePresentMode;
			return;
		}
	}

	MTN_INFO("VSYNC MODE");
	swapChainPresentMode = vk::PresentModeKHR::eFifo;
}

void SnakeApp::ChooseSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX) 
	{
		swapChainExtent = capabilities.currentExtent;
		return;
	}

	size_t width, height;
	vkfw::Result result = window.getFramebufferSize(&width, &height);

	size_t attempts = 0;
	while(result != vkfw::Result::eSuccess && attempts < 200)
	{
		result = window.getFramebufferSize(&width, &height);
		MTN_INFO("Failed attempt {0} at getting framebuffer size, error code: {1}", attempts, vkfw::to_string(result));
		++attempts;
	}

	VkExtent2D actualExtent = {
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};

	actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	swapChainExtent = actualExtent;
}

vk::DebugUtilsMessengerCreateInfoEXT FillDebugMessengerCreateInfo()
{
	vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose| vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
	createInfo.pfnUserCallback = debugCallback;


	;
	return createInfo;
}


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT _messageSeverity,
													VkDebugUtilsMessageTypeFlagsEXT _messageType,
													const VkDebugUtilsMessengerCallbackDataEXT* _pCallbackData,
													void* pUserData) 
{
	vk::DebugUtilsMessageSeverityFlagsEXT messageSeverity(_messageSeverity);
	vk::DebugUtilsMessageTypeFlagsEXT messageType(_messageType);
	vk::DebugUtilsMessengerCallbackDataEXT CallbackData(*_pCallbackData);
	//if(messageSeverity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose) MTN_TRACE("validation layer: {0}", CallbackData.pMessage);
	if(messageSeverity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo) MTN_INFO("validation layer: {0}", CallbackData.pMessage);
	if(messageSeverity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) MTN_WARN("validation layer: {0}", CallbackData.pMessage);
	if(messageSeverity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) MTN_ERROR("validation layer: {0}", CallbackData.pMessage);
	
	return VK_FALSE;
}

glm::vec2 SnakeApp::GenerateApple()
{
	static std::random_device random_device;
	static std::mt19937 random_engine(random_device());
	static std::uniform_real_distribution<float> distribution_1_MaxGrid(1, config.MaxGrid);

	float x = distribution_1_MaxGrid(random_engine);
	float y = distribution_1_MaxGrid(random_engine);
	float divider = 1 / (float)(config.MaxGrid) * (float)(config.gridCount);



	x *= divider;
	y *= divider;
	


	return glm::floor(glm::vec2{x, y});
}