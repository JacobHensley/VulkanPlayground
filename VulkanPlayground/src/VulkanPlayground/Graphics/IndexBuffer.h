#pragma once
#include "VulkanPlayground/Graphics/VertexBuffer.h"
#include "VulkanAllocator.h"
#include <vulkan/vulkan.h>

namespace VKPlayground {

	class IndexBuffer
	{
	public:
		IndexBuffer(void* vertexData, uint32_t size);
		~IndexBuffer();

	public:
		VkBuffer GetVulkanBuffer() { return m_BufferInfo.Buffer; }

	private:
		BufferInfo m_BufferInfo;
	};

}