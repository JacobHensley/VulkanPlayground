#pragma once
#include "VulkanPlayground/Core/VulkanTools.h"
#include "VulkanAllocator.h"
#include <string>

namespace VKPlayground {

	struct ImageInfo
	{
		VkImage Image = nullptr;
		VkImageView ImageView = nullptr;
		VkSampler Sampler = nullptr;
		VmaAllocation MemoryAlloc = nullptr;
	};

	class Texture2D
	{
	public:
		Texture2D(const std::string& path);
		~Texture2D();

		const VkDescriptorImageInfo& GetDescriptorImageInfo() const { return m_DescriptorImageInfo; }

	private:
		std::string m_Path;

		ImageInfo m_ImageInfo;
		VkDescriptorImageInfo m_DescriptorImageInfo;
		
		uint8_t* m_LocalData = nullptr;
		uint32_t m_Width = 0, m_Height = 0;
	};

}
