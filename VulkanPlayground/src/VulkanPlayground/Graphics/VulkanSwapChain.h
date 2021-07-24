#pragma once
#include "VulkanDevice.h"

namespace VKPlayground {

	class VulkanSwapChain
	{
	public:
		VulkanSwapChain();
		~VulkanSwapChain();

	private:
		void Init();

	private:
		VkSwapchainKHR m_SwapChain;

		std::vector<VkImage> m_SwapChainImages;
		std::vector<VkImageView> m_SwapChainImageViews;

		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;
	};

}