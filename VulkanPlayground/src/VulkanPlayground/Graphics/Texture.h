#pragma once

#include <string>
#include "VulkanPlayground/Core/VulkanTools.h"

#include "VulkanAllocator.h"

namespace VKPlayground {

	class Texture2D
	{
	public:
		Texture2D(const std::string& path);
		~Texture2D();

		const VkDescriptorImageInfo& GetDescriptorImageInfo() const { return m_DescriptorImageInfo; }
	private:
		std::string m_Path;
		VkDescriptorImageInfo m_DescriptorImageInfo;

		uint8_t* m_LocalData = nullptr;
		uint32_t m_Width = 0, m_Height = 0;

		struct ImageInfo
		{
			VkImage Image = nullptr;
			VkImageView ImageView = nullptr;
			VkSampler Sampler = nullptr;
			VmaAllocation MemoryAlloc = nullptr;
		} m_ImageInfo;
	};

}
