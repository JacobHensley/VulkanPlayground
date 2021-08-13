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
		void BeginFrame();
		void Present();

		inline VkSwapchainKHR GetSwapChainHandle() { return m_SwapChain; }
		inline VkRenderPass GetRenderPass() { return m_RenderPass; }

		inline const std::vector<VkFramebuffer>& GetFramebuffers() { return m_Framebuffers; }
		VkFramebuffer GetCurrentFramebuffer() const { return m_Framebuffers[m_CurrentImageIndex]; }

		inline std::vector<VkCommandBuffer>& GetCommandBuffers() { return m_CommandBuffers; }
		VkCommandBuffer GetCurrentCommandBuffer() const { return m_CommandBuffers[m_CurrentBufferIndex]; }
	
		inline VkExtent2D GetExtent() { return m_Extent; }

	private:
		void Init();
		void Destroy();

		void PickDetails();
		void CreateImageViews();
		void CreateFramebuffers();
		void CreateCommandBuffers();
		void CreateSynchronizationObjects();

		void Resize();
		VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore);

	private:
		VkSwapchainKHR m_SwapChain = nullptr;
		VkRenderPass m_RenderPass = nullptr;

		VkCommandPool m_CommandPool = nullptr;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		std::vector<SwapChainImage> m_Images;
		std::vector<VkFramebuffer> m_Framebuffers;

		uint32_t m_CurrentImageIndex = 0;
		uint32_t m_CurrentBufferIndex = 0;

		std::vector<VkSemaphore> m_PresentCompleteSemaphores;
		std::vector<VkSemaphore> m_RenderCompleteSemaphores;
		std::vector<VkFence> m_WaitFences;

		VkSurfaceFormatKHR m_ImageFormat;
		VkPresentModeKHR m_PresentMode;
		VkExtent2D m_Extent;
		uint32_t m_ImageCount;
	};

}