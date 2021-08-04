#pragma once
#include "VulkanAllocator.h"
#include <vulkan/vulkan.h>

namespace VKPlayground {

	struct BufferInfo
	{
		VkBuffer Buffer = nullptr;
		VmaAllocation Allocation = nullptr;
	};
	
	class VertexBuffer
	{
	public:
		VertexBuffer(void* vertexData, uint32_t size);
		~VertexBuffer();

	public:
		VkBuffer GetVulkanBuffer() { return m_BufferInfo.Buffer; }

	private:
		BufferInfo m_BufferInfo;
	};


}