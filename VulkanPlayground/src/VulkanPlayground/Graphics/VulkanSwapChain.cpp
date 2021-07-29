#include "pch.h"
#include "VulkanSwapChain.h"
#include "VulkanPlayground/Core/Application.h"
#include <glm/glm.hpp>

namespace VKPlayground {

	VulkanSwapChain::VulkanSwapChain()
	{
		Init();
		LOG_INFO("Initialized Vulkan swap chain");
	}

	VulkanSwapChain::~VulkanSwapChain()
	{
		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();

		for (auto framebuffer : m_Framebuffers)
		{
			vkDestroyFramebuffer(device->GetLogicalDevice(), framebuffer, nullptr);
		}

		vkDestroyRenderPass(device->GetLogicalDevice(), m_RenderPass, nullptr);

		for (auto image : m_Images)
		{
			vkDestroyImageView(device->GetLogicalDevice(), image.ImageView, nullptr);
		}

		vkDestroySwapchainKHR(device->GetLogicalDevice(), m_SwapChain, nullptr);
	}

	void VulkanSwapChain::Init()
	{
		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();
		VkSurfaceKHR surface = Application::GetApp().GetWindow()->GetVulkanSurface();
		SwapChainSupportDetails supportDetails = device->GetSwapChainSupportDetails();

		PickDetails();

		// Create swapchain info
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = m_ImageCount;
		createInfo.imageFormat = m_ImageFormat.format;
		createInfo.imageColorSpace = m_ImageFormat.colorSpace;
		createInfo.presentMode = m_PresentMode;
		createInfo.imageExtent = m_Extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.preTransform = supportDetails.Capabilities.currentTransform;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		// Select exclusive vs concurrent mode
		QueueFamilyIndices indices = device->GetQueueIndices();
		uint32_t queueFamilyIndices[] = { indices.GraphicsQueue.value(), indices.PresentQueue.value() };
		if (indices.GraphicsQueue != indices.PresentQueue)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else 
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		// Create swap chain
		VkResult result = vkCreateSwapchainKHR(device->GetLogicalDevice(), &createInfo, nullptr, &m_SwapChain);
		ASSERT(result == VK_SUCCESS, "Failed to initialize Vulkan swap chain");

		// Get swap chain image handles
		vkGetSwapchainImagesKHR(device->GetLogicalDevice(), m_SwapChain, &m_ImageCount, nullptr);
		m_Images.resize(m_ImageCount);

		std::vector<VkImage> images(m_ImageCount);
		vkGetSwapchainImagesKHR(device->GetLogicalDevice(), m_SwapChain, &m_ImageCount, images.data());

		for (int i = 0; i < m_ImageCount; i++)
			m_Images[i].Image = images[i];

		// Create image views
		for (int i = 0; i < m_Images.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_Images[i].Image;

			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_ImageFormat.format;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			
			VkResult result = vkCreateImageView(device->GetLogicalDevice(), &createInfo, nullptr, &m_Images[i].ImageView);
			ASSERT(result == VK_SUCCESS, "Failed to initialize Vulkan image view");
		}

		CreateFramebuffers();
	}

	void VulkanSwapChain::PickDetails()
	{
		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();
		SwapChainSupportDetails supportDetails = device->GetSwapChainSupportDetails();

		// Select format
		std::vector<VkSurfaceFormatKHR> formats = supportDetails.Formats;
		VkSurfaceFormatKHR selectedFormat = formats[0];
		for (const auto& availableFormat : formats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				selectedFormat = availableFormat;
			}
		}

		// Select present mode
		std::vector<VkPresentModeKHR> presentModes = supportDetails.PresentModes;
		VkPresentModeKHR selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const auto& availablePresentMode : presentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				selectedPresentMode = availablePresentMode;
			}
		}

		// Select extent
		VkSurfaceCapabilitiesKHR capabilities = supportDetails.Capabilities;
		VkExtent2D selectedExtent;
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			selectedExtent = capabilities.currentExtent;
		}
		else
		{
			glm::ivec2 windowSizePixels = Application::GetApp().GetWindow()->GetFramebufferSize();
			VkExtent2D actualExtent =
			{
				static_cast<uint32_t>(windowSizePixels.x),
				static_cast<uint32_t>(windowSizePixels.y)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			selectedExtent = actualExtent;
		}

		// Select image count
		uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		{
			imageCount = capabilities.maxImageCount;
		}

		m_ImageFormat = selectedFormat;
		m_PresentMode = selectedPresentMode;
		m_Extent = selectedExtent;
		m_ImageCount = imageCount;
	}

	void VulkanSwapChain::CreateFramebuffers()
	{
		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();

		// Color attachment
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_ImageFormat.format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Reference to color attachment
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// Create subpass using color attachment reference
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		// Create subpass for render pass that waits for the swapchain to be done reading the image
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		// Create render pass
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(device->GetLogicalDevice(), &renderPassInfo, nullptr, &m_RenderPass);
		ASSERT(result == VK_SUCCESS, "Failed to initialize Vulkan render pass");

		// Create framebuffer for each image in the swap chain
		m_Framebuffers.resize(m_Images.size());
		for (size_t i = 0; i < m_Images.size(); i++)
		{
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &m_Images[i].ImageView;
			framebufferInfo.width = m_Extent.width;
			framebufferInfo.height = m_Extent.height;
			framebufferInfo.layers = 1;

			result = vkCreateFramebuffer(device->GetLogicalDevice(), &framebufferInfo, nullptr, &m_Framebuffers[i]);
			ASSERT(result == VK_SUCCESS, "Failed to initialize Vulkan framebuffer");
		}
	}

}