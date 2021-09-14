#include "pch.h"
#include "Texture.h"
#include "VulkanPlayground/Core/Application.h"
#include <stb/stb_image.h>

namespace VKPlayground {

	Texture2D::Texture2D(const std::string& path)
		: m_Path(path)
	{
		// Load image from disk
		int width, height, bpp;
		stbi_set_flip_vertically_on_load(true);
		m_LocalData = stbi_load(path.c_str(), &width, &height, &bpp, 4);
		ASSERT(m_LocalData, "Failed to load image");

		// Set size, width, and height
		uint32_t size = width * height * 4;
		m_Width = width;
		m_Height = height;

		// Create staging buffer with image data
		VulkanBuffer stagingBuffer(m_LocalData, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		// Free CPU memory
		stbi_image_free(m_LocalData);

		// Image create info
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageCreateInfo.extent.width = m_Width;
		imageCreateInfo.extent.height = m_Height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		// Allocate and create image object
		VulkanAllocator allocator("Texture2D");
		m_ImageInfo.MemoryAlloc = allocator.AllocateImage(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_ImageInfo.Image);

		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();
		VkCommandBuffer commandBuffer = device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		// Range of image to copy (only the first mip and first layer)
		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		// Transfer image from undefined layout to destination optimal for copying into
		InsertImageMemoryBarrier(
			commandBuffer, 
			m_ImageInfo.Image, 
			0, 
			VK_ACCESS_TRANSFER_WRITE_BIT, 
			VK_IMAGE_LAYOUT_UNDEFINED, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, 
			range);

		// Define what part of the image to copy
		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;
		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent.width = m_Width;
		copyRegion.imageExtent.height = m_Height;
		copyRegion.imageExtent.depth = 1;

		// Copy CPU-GPU buffer into GPU-ONLY texture
		vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.GetVulkanBuffer(), m_ImageInfo.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		// Transfer image from destination optimal layout to shader read optimal
		InsertImageMemoryBarrier(
			commandBuffer,
			m_ImageInfo.Image,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			range);

		// Submit and free command buffer
		device->FlushCommandBuffer(commandBuffer, true);

		// Create image view (only the first mip and first layer)
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.subresourceRange = {};
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.image = m_ImageInfo.Image;

		VK_CHECK_RESULT(vkCreateImageView(device->GetLogicalDevice(), &imageViewCreateInfo, nullptr, &m_ImageInfo.ImageView));

		// Create sampler
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.anisotropyEnable = VK_FALSE;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 20.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;

		VK_CHECK_RESULT(vkCreateSampler(device->GetLogicalDevice(), &samplerCreateInfo, nullptr, &m_ImageInfo.Sampler));

		// Create descriptor image info
		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView = m_ImageInfo.ImageView;
		m_DescriptorImageInfo.sampler = m_ImageInfo.Sampler;
	}

	Texture2D::~Texture2D()
	{
		VulkanAllocator allocator("Texture2D");
		allocator.DestroyImage(m_ImageInfo.Image, m_ImageInfo.MemoryAlloc);

		VkDevice device = Application::GetApp().GetVulkanDevice()->GetLogicalDevice();
		vkDestroyImageView(device, m_ImageInfo.ImageView, nullptr);
		vkDestroySampler(device, m_ImageInfo.Sampler, nullptr);
	}

}