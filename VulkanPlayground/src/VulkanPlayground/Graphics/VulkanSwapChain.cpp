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

		vkDestroyRenderPass(device->GetLogicalDevice(), m_RenderPass, nullptr);

		for (auto imageView : m_SwapChainImageViews)
		{
			vkDestroyImageView(device->GetLogicalDevice(), imageView, nullptr);
		}

		vkDestroySwapchainKHR(device->GetLogicalDevice(), m_SwapChain, nullptr);
	}

	void VulkanSwapChain::Init()
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

		VkSurfaceKHR surface = Application::GetApp().GetWindow()->GetVulkanSurface();

		// Create swapchain info
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = selectedFormat.format;
		createInfo.imageColorSpace = selectedFormat.colorSpace;
		createInfo.presentMode = selectedPresentMode;
		createInfo.imageExtent = selectedExtent;
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
		vkGetSwapchainImagesKHR(device->GetLogicalDevice(), m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);

		vkGetSwapchainImagesKHR(device->GetLogicalDevice(), m_SwapChain, &imageCount, m_SwapChainImages.data());

		// Store swap chain settings
		m_SwapChainImageFormat = selectedFormat.format;
		m_SwapChainExtent = selectedExtent;

		// Create image views
		m_SwapChainImageViews.resize(m_SwapChainImages.size());
		for (int i = 0; i < m_SwapChainImageViews.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_SwapChainImages[i];

			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_SwapChainImageFormat;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			
			VkResult result = vkCreateImageView(device->GetLogicalDevice(), &createInfo, nullptr, &m_SwapChainImageViews[i]);
			ASSERT(result == VK_SUCCESS, "Failed to initialize Vulkan image view");
		}

		// Create render pass and sub pass

		// Color attachment
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_SwapChainImageFormat;
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

		// Create render pass
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		result = vkCreateRenderPass(device->GetLogicalDevice(), &renderPassInfo, nullptr, &m_RenderPass);
		ASSERT(result == VK_SUCCESS, "Failed to initialize Vulkan render pass");
	}

}