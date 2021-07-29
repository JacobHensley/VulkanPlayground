#pragma once
#include "VulkanDevice.h"
#include "VulkanPipline.h"
#include <Vulkan/vulkan.h>

namespace VKPlayground {

	struct SwapChainImage
	{
		VkImage Image;
		VkImageView ImageView;
	};

	class VulkanSwapChain
	{
	public:
		VulkanSwapChain();
		~VulkanSwapChain();

	public:
		inline VkSwapchainKHR GetSwapChainHandle() { return m_SwapChain; }
		inline VkRenderPass GetRenderPass() { return m_RenderPass; }
		inline const std::vector<VkFramebuffer>& GetFramebuffers() { return m_Framebuffers; }

		inline VkExtent2D GetExtent() { return m_Extent; }

	private:
		void Init();
		void PickDetails();
		void CreateFramebuffers();

	private:
		VkSwapchainKHR m_SwapChain;
		VkRenderPass m_RenderPass;

		std::vector<SwapChainImage> m_Images;
		std::vector<VkFramebuffer> m_Framebuffers;

		VkSurfaceFormatKHR m_ImageFormat;
		VkPresentModeKHR m_PresentMode;
		VkExtent2D m_Extent;
		uint32_t m_ImageCount;
	};

}