#pragma once
#include "Core.h"
#include <string>
#include <Vulkan/vulkan.h>

#define VK_CHECK_RESULT(f)																					\
{																											\
	VkResult vk_error_result = (f);																			\
	ASSERT(vk_error_result == VK_SUCCESS, "Vulkan Error: VK_" + VulkanErrorString(vk_error_result));		\
}

namespace VKPlayground {

	std::string VulkanErrorString(VkResult errorCode);

	void InsertImageMemoryBarrier(
		VkCommandBuffer commandBuffer,
		VkImage image,
		VkAccessFlags srcAccessMask,
		VkAccessFlags dstAccessMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask,
		VkImageSubresourceRange subresourceRange);
}