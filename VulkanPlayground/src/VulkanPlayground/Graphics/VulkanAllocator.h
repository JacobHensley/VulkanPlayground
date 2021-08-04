#pragma once
#include "VulkanPlayground/Core/Core.h"
#include "VulkanDevice.h"
#include <vk_mem_alloc.h>

namespace VKPlayground {

	class VulkanAllocator
	{
	public:
		VulkanAllocator(const std::string& tag);
		~VulkanAllocator();
	
	public:
		VmaAllocation AllocateBuffer(const VkBufferCreateInfo& bufferCreateInfo, VmaMemoryUsage usage, VkBuffer& outBuffer);
		void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);

		template<typename T>
		T* MapMemory(VmaAllocation allocation)
		{
			T* outData;
			vmaMapMemory(GetVMAAllocator(), allocation, (void**)&outData);
			return outData;
		}

		void UnmapMemory(VmaAllocation allocation);

	public:
		static void Init(Ref<VulkanDevice> device);
		static void Shutdown();

		static VmaAllocator& GetVMAAllocator();

	private:
		std::string m_Tag;
	};

}
