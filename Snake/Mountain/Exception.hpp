#pragma once
#ifndef CC74D858_8536_42BC_9BD1_C164A6D928CE
#define CC74D858_8536_42BC_9BD1_C164A6D928CE

#include "Result.hpp"
#include <string>
#include <stdexcept>

#define VULKAN_HPP_NO_EXCEPTIONS 
#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC != 1
	#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
	#include <vulkan/vulkan.hpp>
#else
	#include <vulkan/vulkan.hpp>
#endif

namespace Mountain 
{

	struct Exception : public std::exception
	{
		Exception(const std::string& message)
			: std::exception(message.c_str())
		{

		}

		Exception(const std::string& message, vkfw::Result result)
			: Exception((std::string("Mountain Exception (vkfw): ") + message + "(Error message: " + vkfw::to_string(result) + ", Error code: " + std::to_string((int) result) + ")"))
		{

		}

		Exception(const std::string& message, vk::Result result)
			: Exception((std::string("Mountain Exception (vulkan): ") + message + "(Error message: " + vk::to_string(result) + ", Error code: " + std::to_string((int) result) + ")"))
		{

		}

		Exception(const std::string& message, Result result)
			: Exception((std::string("Mountain Exception: ") + message + "(Error message: " + Mountain::to_string(result) + ", Error code: " + std::to_string((int) result) + ")"))
		{

		}

		void operator()(const std::string& message, Result result)
		{
			*this = Exception(message, result);
		}

		void operator()(const std::string& message, vk::Result result)
		{
			*this = Exception(message, result);
		}

		
		void operator()(const std::string& message, vkfw::Result result)
		{
			*this = Exception(message, result);
		}


	};

}

#endif /* CC74D858_8536_42BC_9BD1_C164A6D928CE */
