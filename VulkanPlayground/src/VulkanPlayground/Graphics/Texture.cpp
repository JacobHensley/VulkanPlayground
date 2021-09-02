#include "pch.h"
#include "Texture.h"

#include "stb/stb_image.h"

namespace VKPlayground {

	Texture2D::Texture2D(const std::string& path)
		: m_Path(path)
	{

		int width, height, bpp;
		m_LocalData = stbi_load(path.c_str(), &width, &height, &bpp, 4);
		if (!m_LocalData)
		{
			LOG_INFO("Failed to load image!");
			return;
		}

		uint32_t size = width * height * 4;
		m_Width = width;
		m_Height = height;

		bool useStagingBuffers = true;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.extent.width = m_Width;
		imageCreateInfo.extent.height = m_Height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1; // TODO
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = usage;

		VulkanAllocator allocator("Texture2D");
		m_ImageInfo.MemoryAlloc = allocator.AllocateImage(imageCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_ImageInfo.Image);
		
		// Free CPU RAM
		stbi_image_free(m_LocalData);
	}

	Texture2D::~Texture2D()
	{

	}

}