#include "pch.h"
#include "VulkanBuffers.h"

namespace VKPlayground {

	VulkanVertexBuffer::VulkanVertexBuffer(void* vertexData, uint32_t size)
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

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		VulkanAllocator allocator("VertexBuffer");
		allocator.DestroyBuffer(m_BufferInfo.Buffer, m_BufferInfo.Allocation);
	}

	VulkanIndexBuffer::VulkanIndexBuffer(void* indexData, uint32_t size)
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
		memcpy(dstBuffer, indexData, size);
		allocator.UnmapMemory(m_BufferInfo.Allocation);
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		VulkanAllocator allocator("IndexBuffer");
		allocator.DestroyBuffer(m_BufferInfo.Buffer, m_BufferInfo.Allocation);
	}

	VulkanUniformBuffer::VulkanUniformBuffer(void* data, uint32_t size)
	{
		// Create buffer info
		VkBufferCreateInfo vertexBufferCreateInfo = {};
		vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferCreateInfo.size = size;
		vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		// Allocate memory
		VulkanAllocator allocator("UniformBuffer");
		m_BufferInfo.Allocation = allocator.AllocateBuffer(vertexBufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_BufferInfo.Buffer);

		// Copy data into buffer
		void* dstBuffer = allocator.MapMemory<void>(m_BufferInfo.Allocation);
		memcpy(dstBuffer, data, size);
		allocator.UnmapMemory(m_BufferInfo.Allocation);

		m_DescriptorBufferInfo.buffer = m_BufferInfo.Buffer;
		m_DescriptorBufferInfo.offset = 0;
		m_DescriptorBufferInfo.range = VK_WHOLE_SIZE;
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		VulkanAllocator allocator("UniformBuffer");
		allocator.DestroyBuffer(m_BufferInfo.Buffer, m_BufferInfo.Allocation);
	}

	VulkanStagingBuffer::VulkanStagingBuffer(void* data, uint32_t size)
	{
		// Create buffer info
		VkBufferCreateInfo vertexBufferCreateInfo = {};
		vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferCreateInfo.size = size;
		vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		// Allocate memory
		VulkanAllocator allocator("StagingBuffer");
		m_BufferInfo.Allocation = allocator.AllocateBuffer(vertexBufferCreateInfo, VMA_MEMORY_USAGE_CPU_ONLY, m_BufferInfo.Buffer);

		// Copy data into buffer
		void* dstBuffer = allocator.MapMemory<void>(m_BufferInfo.Allocation);
		memcpy(dstBuffer, data, size);
		allocator.UnmapMemory(m_BufferInfo.Allocation);
	}

	VulkanStagingBuffer::~VulkanStagingBuffer()
	{
		VulkanAllocator allocator("StagingBuffer");
		allocator.DestroyBuffer(m_BufferInfo.Buffer, m_BufferInfo.Allocation);
	}

}