#pragma once
#include "Core.h"
#include <string>
#include <Vulkan/vulkan.h>

namespace VKPlayground {

	#define VK_CHECK_RESULT(f)																\
	{																						\
		VkResult result = (f);																\
		ASSERT(result == VK_SUCCESS, "Vulkan Error: VK_" + VulkanErrorString(result));		\
	}

	std::string VulkanErrorString(VkResult errorCode);

}