#include "SnakeApp.hpp"

#define VULKAN_HPP_NO_EXCEPTIONS 
#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC != 1
	#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
	#include <vulkan/vulkan.hpp>
	VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#else
	#include <vulkan/vulkan.hpp>
#endif

#include "Mountain/Logger.hpp"
#include <fstream>

int main()
{
	std::cout << "Hello World";
	
	SnakeApp snake ([](Mountain::Exception error){ MTN_CRITICAL(error.what()); std::ofstream os("./error.log"); os << error.what(); });

	if(!snake.Initialize()) 
	{
		std::cout << "Failed to initialize snake." << std::endl;
		std::cin.get();
		return EXIT_FAILURE;

	}

	return EXIT_SUCCESS;
}
