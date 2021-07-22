#ifndef EDD4DF62_E55E_4870_868B_D5BE13BFF952
#define EDD4DF62_E55E_4870_868B_D5BE13BFF952

#include "Mountain/Instance.hpp"
#include "Mountain/Window.hpp"
#include "Mountain/Exception.hpp"
#include "Mountain/Device.hpp"
#include "Mountain/StringUtil.hpp"
#include "Mountain/Logger.hpp"

#include <functional>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct SnakeApp 
{
	SnakeApp(std::function<void(Mountain::Exception)> _errorHandler = nullptr)
		: errorHandler{_errorHandler}
	{}

	bool Initialize();

	~SnakeApp() { DestroyAll(); }

private:


	bool InitGLFW();
	void GetConfig();
	void InitApp();
	bool CreateInstance();
	bool CreateWindowHandle();
	void CreateWindowSurface();
	bool CreateDevice();
	bool CreateSwapChain();

	// Swapchain utility functions

	void ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>&);
	void ChooseSwapChainPresentMode(const std::vector<vk::PresentModeKHR> &);
	void ChooseSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

	bool CreateImageViews();
	bool CreateGraphicsPipeline();
	
	// Pipeline utility functions

	std::vector<char> ReadShader(const std::string& fileName);
	vk::ResultValue<vk::ShaderModule> CreateShaderModule(const std::string& fileName);
	vk::ResultValue<vk::ShaderModule> CreateShaderModule(const std::vector<char>& code);
	vk::Viewport GetViewport();
	vk::Rect2D GetScissor();
	vk::PipelineVertexInputStateCreateInfo 	 GetVertexInputInfo();
	vk::PipelineInputAssemblyStateCreateInfo GetInputAssemblyInfo();
	vk::PipelineRasterizationStateCreateInfo GetRasterizationInfo();
	vk::PipelineMultisampleStateCreateInfo   GetMultisampleingInfo();
	vk::PipelineColorBlendAttachmentState    GetColorBlendAttachment();
	vk::PipelineColorBlendStateCreateInfo    GetColorBlendInfo(vk::PipelineColorBlendAttachmentState& colorBlendAttachment);
	bool CreatePipelineLayout();
	bool CreateRenderPass();

	bool CreateFramebuffers();
	bool CreateCommandPool();
	bool AllocateCommandBuffers();
	bool RecreateCommandBuffer();
	bool CreateSyncObjects();
	bool CreateVertexBuffer();
	bool CreateIndexBuffer();
	bool CreateUniformBuffer();
	bool CreateDescriptorPool();

	// Buffer Utility Function
	bool CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);
	bool CreateStagingBuffer(size_t size, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);
	bool CopyBuffer(size_t size, vk::Buffer& src, vk::Buffer& dst);
	std::tuple<bool, uint32_t> FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

	// Index Buffer Utility function
	void FillIndices();
	bool SubmitIndices();

	// Vertex Buffer Utility function
	void FillVertices();
	bool SubmitVertices();

	// Uniform Buffer Utility Function
	bool SubmitUniform();


	bool CreateDescriptorSetLayout();
	bool CreateDescriptorSet();

	void HandleInput(vkfw::Window window, vkfw::Key key, int32_t scancode, vkfw::KeyAction action, vkfw::ModifierKeyFlags flags);
	bool Run();
	bool Update(const double& deltaTime);
	bool Draw();

	void CleanupSwapChain();
	bool RecreateSwapChain();


	

private:

	void DestroyAll();

	void DestroySwapChain();



private:

	glm::vec2 GenerateApple();
	

private:

struct Vertex
{
	glm::vec2 position;
	glm::vec3 color;
	float IsCircle = 0.0;

	
	static vk::VertexInputBindingDescription GetBindingDescription() {
		vk::VertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = vk::VertexInputRate::eVertex;
		return bindingDescription;
	}

	
	static std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescriptions() {
		std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = vk::Format::eR32Sfloat;
		attributeDescriptions[2].offset = offsetof(Vertex, IsCircle);

		return attributeDescriptions;
	}


};

struct Uniform 
{
	glm::mat4 mvp;
	glm::vec4 viewport;

	// Apple will always be in center of quads and there will always be a 0.5 but 0.5 would make some pixels invisible so instead i chose to put 0.4 from center to border of quad.
	const float radius = 0.4;

	// Apple - vec2(radius, radius). Will be multiplied by MVP in fragment shader, should be in the center of quad.
	glm::vec2 circleCenter;

	void SetViewport(vk::Viewport _viewport)
	{
		viewport = { _viewport.x, _viewport.y, _viewport.width, _viewport.height };
	}
};

enum class Direction
{
	Left, Right, Up, Down, None
};


struct SnakeData
{
	glm::vec2 head;
	Direction direction = Direction::None, input = Direction::None;
	size_t tailSize = 4, defaultSize = 4, increment = 1;

	struct TailPart
	{
		glm::vec2 position;
		Direction direction;
	};

	std::vector<TailPart> tail;
	glm::vec2 applePos;
	constexpr static glm::vec3 color = {0.101, 0.6, 0};
	constexpr static glm::vec3 appleColor = {0.6, 0.0, 0.0};
	constexpr static float speed = 15;
};

private:

struct AppConfig 
{
	size_t width = 400, height = 400;
	
	uint16_t gridCount = 20;

	bool fullscreen = false;

	static constexpr uint32_t MaxGrid = 80;
	static constexpr uint32_t MaxVertices = MaxGrid * MaxGrid * 4;
	static constexpr uint32_t MaxVerticesSize = MaxVertices * sizeof(Vertex);

	static constexpr uint32_t MaxIndices = MaxGrid * MaxGrid * 6;
	static constexpr uint32_t MaxIndicesSize = MaxIndices * sizeof(uint32_t);
};

private:

	std::function<void(Mountain::Exception)> errorHandler;
	AppConfig config;
	vkfw::Window window;
	vk::Instance instance;
	vk::DebugUtilsMessengerEXT debugMessenger;
	vk::SurfaceKHR surface;
	vk::PhysicalDevice physicalDevice;
	vk::Device device;
	Mountain::DefaultQueues queues;
	vk::SwapchainKHR swapChain;
	vk::SurfaceFormatKHR surfaceFormat;
	vk::PresentModeKHR swapChainPresentMode;
	vk::Extent2D swapChainExtent;
	vk::Format swapChainImageFormat;
	std::vector<vk::Image> images;
	std::vector<vk::ImageView> imageViews;
	vk::Pipeline graphicsPipeline;
	vk::DescriptorSetLayout descriptorSetLayout;
	vk::PipelineLayout pipelineLayout;
	vk::RenderPass renderPass;
	std::vector<vk::Framebuffer> swapChainFramebuffers;
	vk::CommandPool commandPool;
	std::vector<vk::CommandBuffer> commandBuffers;
	std::vector<vk::Semaphore> imageAvailableSemaphores;
	std::vector<vk::Semaphore> renderFinishedSemaphores;
	std::vector<vk::Fence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	vk::DescriptorPool descriptorPool;
	vk::DescriptorSet descriptorSet;
	

	vk::Buffer vertexBuffer, indexBuffer, uniformBuffer, stagingBuffer;
	vk::DeviceMemory vertexBufferMemory, indexBufferMemory, uniformBufferMemory, stagingBufferMemory;


	static constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;
	size_t currentFrame = 0;
	bool framebufferResized = false;

	std::vector<Vertex> vertices = std::vector<Vertex>(AppConfig::MaxVertices);
	std::vector<uint32_t> indices = std::vector<uint32_t>(AppConfig::MaxIndices);

	SnakeData snake;
	Uniform uniform;


	static constexpr size_t a = sizeof(Uniform);
};

#endif /* EDD4DF62_E55E_4870_868B_D5BE13BFF952 */