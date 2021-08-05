#pragma once
#include "VulkanDevice.h"
#include "VulkanPipeline.h"
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

		inline std::vector<VkCommandBuffer>& GetCommandBuffers() { return m_CommandBuffers; }
			   
		inline std::vector<VkSemaphore>& GetImageAvailableSemaphores() { return m_ImageAvailableSemaphores; }
		inline std::vector<VkSemaphore>& GetRenderFinishedSemaphores() { return m_RenderFinishedSemaphores; }
			   
		inline std::vector<VkFence>& GetInFlightFences() { return m_InFlightFences; }
		inline std::vector<VkFence>& GetImagesInFlight() { return m_ImagesInFlight; }

	private:
		void Init();
		void PickDetails();
		void CreateImageViews();
		void CreateFramebuffers();
		void CreateCommandBuffers();
		void CreateSynchronizationObjects();

	private:
		VkSwapchainKHR m_SwapChain = nullptr;
		VkRenderPass m_RenderPass = nullptr;

		std::vector<SwapChainImage> m_Images;
		std::vector<VkFramebuffer> m_Framebuffers;

		VkCommandPool m_CommandPool = nullptr;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;

		std::vector<VkFence> m_InFlightFences;
		std::vector<VkFence> m_ImagesInFlight;

		VkSurfaceFormatKHR m_ImageFormat;
		VkPresentModeKHR m_PresentMode;
		VkExtent2D m_Extent;
		uint32_t m_ImageCount;
	};

}