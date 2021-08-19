#pragma once
#include "Core.h"
#include <string>
#include <Vulkan/vulkan.h>

namespace VKPlayground {

	#define VK_CHECK_RESULT(f)																					\
	{																											\
		VkResult vk_error_result = (f);																			\
		ASSERT(vk_error_result == VK_SUCCESS, "Vulkan Error: VK_" + VulkanErrorString(vk_error_result));		\
	}

	std::string VulkanErrorString(VkResult errorCode);

}