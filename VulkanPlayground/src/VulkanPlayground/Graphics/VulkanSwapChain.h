#pragma once
#include "VulkanDevice.h"
#include "VulkanPipline.h"
#include <Vulkan/vulkan.h>

namespace VKPlayground {

	class VulkanSwapChain
	{
	public:
		VulkanSwapChain();
		~VulkanSwapChain();

	public:
		inline VkExtent2D GetExtent() { return m_SwapChainExtent; }
		inline VkRenderPass GetRenderPass() { return m_RenderPass; }
		inline const std::vector<VkFramebuffer>& GetFramebuffers() { return m_SwapChainFramebuffers; }

	private:
		void Init();

	private:
		VkSwapchainKHR m_SwapChain;

		std::vector<VkImage> m_SwapChainImages;
		std::vector<VkImageView> m_SwapChainImageViews;

		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;

		VkRenderPass m_RenderPass;
	};

}