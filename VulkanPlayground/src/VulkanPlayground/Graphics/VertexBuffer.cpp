#include "pch.h"
#include "VertexBuffer.h"

namespace VKPlayground {

	VertexBuffer::VertexBuffer(void* vertexData, uint32_t size)
	{
		// Create buffer info
		VkBufferCreateInfo vertexBufferCreateInfo = {};
		vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferCreateInfo.size = size;
		vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		vertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		// Allocate memory
		VulkanAllocator allocator("VertexBuffer");
		m_BufferInfo.Allocation = allocator.AllocateBuffer(vertexBufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_BufferInfo.Buffer);

		// Copy data into buffer
		void* dstBuffer = allocator.MapMemory<void>(m_BufferInfo.Allocation);
		memcpy(dstBuffer, vertexData, size);
		allocator.UnmapMemory(m_BufferInfo.Allocation);
	}

	VertexBuffer::~VertexBuffer()
	{
		VulkanAllocator allocator("VertexBuffer");
		allocator.DestroyBuffer(m_BufferInfo.Buffer, m_BufferInfo.Allocation);
	}

}