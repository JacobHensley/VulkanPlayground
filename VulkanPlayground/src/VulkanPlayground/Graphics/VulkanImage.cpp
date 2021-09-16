#include "pch.h"
#include "VulkanImage.h"
#include "VulkanPlayground/Core/Application.h"

namespace VKPlayground {

	VulkanImage::VulkanImage(ImageSpecification specification)
		:	m_Specification(specification)
	{
		Init();
	}

	VulkanImage::~VulkanImage()
	{
		VulkanAllocator allocator("Texture2D");
		allocator.DestroyImage(m_ImageInfo.Image, m_ImageInfo.MemoryAlloc);

		VkDevice device = Application::GetApp().GetVulkanDevice()->GetLogicalDevice();
		vkDestroyImageView(device, m_ImageInfo.ImageView, nullptr);
		vkDestroySampler(device, m_ImageInfo.Sampler, nullptr);
	}

	void VulkanImage::Init()
	{
		// TODO: Obtain BPP from image format
		uint32_t size = m_Specification.Width * m_Specification.Height * 4;

		// TODO: Ask Yan about usage
		// Image create info
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = m_Specification.Format;
		imageCreateInfo.extent.width = m_Specification.Width;
		imageCreateInfo.extent.height = m_Specification.Height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = m_Specification.LayerCount;
		imageCreateInfo.samples = m_Specification.SampleCount;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = m_Specification.Usage | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		// Create staging buffer with image data	
		VulkanBuffer stagingBuffer(m_Specification.Data, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		// Allocate and create image object
		VulkanAllocator allocator("Texture2D");
		m_ImageInfo.MemoryAlloc = allocator.AllocateImage(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_ImageInfo.Image);

		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();
		VkCommandBuffer commandBuffer = device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		VkImageAspectFlags aspectFlag = IsDepthFormat(m_Specification.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		// Range of image to copy (only the first mip)
		VkImageSubresourceRange range;
		range.aspectMask = aspectFlag;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = m_Specification.LayerCount;

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
		copyRegion.imageSubresource.aspectMask = aspectFlag;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = m_Specification.LayerCount;
		copyRegion.imageExtent.width = m_Specification.Width;
		copyRegion.imageExtent.height = m_Specification.Height;
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

		// Create image view
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = m_Specification.Format;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.subresourceRange = {};
		imageViewCreateInfo.subresourceRange.aspectMask = aspectFlag;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = m_Specification.LayerCount;
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

	bool VulkanImage::IsDepthFormat(VkFormat format)
	{
		std::vector<VkFormat> formats =
		{
			VK_FORMAT_D16_UNORM,
			VK_FORMAT_X8_D24_UNORM_PACK32,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
		};

		return std::find(formats.begin(), formats.end(), format) != std::end(formats);
	}

	bool VulkanImage::IsStencilFormat(VkFormat format)
	{
		std::vector<VkFormat> formats =
		{
			VK_FORMAT_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
		};

		return std::find(formats.begin(), formats.end(), format) != std::end(formats);
	}

}