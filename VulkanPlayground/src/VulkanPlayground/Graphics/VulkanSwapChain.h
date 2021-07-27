#pragma once
#include "VulkanDevice.h"

namespace VKPlayground {

	class VulkanSwapChain
	{
	public:
		VulkanSwapChain();
		~VulkanSwapChain();

	public:
		inline VkExtent2D GetExtent() { return m_SwapChainExtent; }
		inline VkRenderPass GetRenderPass() { return m_RenderPass; }

	private:
		void Init();

	private:
		VkSwapchainKHR m_SwapChain;

		std::vector<VkImage> m_SwapChainImages;
		std::vector<VkImageView> m_SwapChainImageViews;

		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;

		VkRenderPass m_RenderPass;
	};

}