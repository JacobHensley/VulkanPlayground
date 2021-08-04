#include "pch.h"
#include "IndexBuffer.h"

namespace VKPlayground {

	IndexBuffer::IndexBuffer(void* vertexData, uint32_t size)
	{
		// Create buffer info
		VkBufferCreateInfo vertexBufferCreateInfo = {};
		vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferCreateInfo.size = size;
		vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		// Allocate memory
		VulkanAllocator allocator("IndexBuffer");
		m_BufferInfo.Allocation = allocator.AllocateBuffer(vertexBufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_BufferInfo.Buffer);

		// Copy data into buffer
		void* dstBuffer = allocator.MapMemory<void>(m_BufferInfo.Allocation);
		memcpy(dstBuffer, vertexData, size);
		allocator.UnmapMemory(m_BufferInfo.Allocation);
	}

	IndexBuffer::~IndexBuffer()
	{
		VulkanAllocator allocator("IndexBuffer");
		allocator.DestroyBuffer(m_BufferInfo.Buffer, m_BufferInfo.Allocation);
	}

}