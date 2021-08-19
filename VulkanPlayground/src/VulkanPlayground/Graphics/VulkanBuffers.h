#pragma once
#include "VulkanAllocator.h"
#include <vulkan/vulkan.h>

namespace VKPlayground {

	struct BufferInfo
	{
		VkBuffer Buffer = nullptr;
		VmaAllocation Allocation = nullptr;
	};

	class VulkanVertexBuffer
	{
	public:
		VulkanVertexBuffer(void* vertexData, uint32_t size);
		~VulkanVertexBuffer();

	public:
		VkBuffer GetVulkanBuffer() { return m_BufferInfo.Buffer; }

	private:
		BufferInfo m_BufferInfo;
	};

	class VulkanIndexBuffer
	{
	public:
		VulkanIndexBuffer(void* indexData, uint32_t size);
		~VulkanIndexBuffer();

	public:
		VkBuffer GetVulkanBuffer() { return m_BufferInfo.Buffer; }

	private:
		BufferInfo m_BufferInfo;
	};

	class VulkanUniformBuffer
	{
	public:
		VulkanUniformBuffer(void* data, uint32_t size);
		~VulkanUniformBuffer();

	public:
		VkBuffer GetVulkanBuffer() { return m_BufferInfo.Buffer; }
		const VkDescriptorBufferInfo& getDescriptorBufferInfo() { return m_DescriptorBufferInfo; }

	private:
		BufferInfo m_BufferInfo;
		VkDescriptorBufferInfo m_DescriptorBufferInfo;
	};

}