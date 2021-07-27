#include "pch.h"
#include "SimpleRenderer.h"
#include "Application.h"
#include "VulkanPlayground/Graphics/VulkanDevice.h"

namespace VKPlayground {

	SimpleRenderer::SimpleRenderer()
	{
		Init();
		LOG_INFO("Initialized renderer");
	}

	SimpleRenderer::~SimpleRenderer()
	{
		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();
		vkDestroyCommandPool(device->GetLogicalDevice(), m_CommandPool, nullptr);
	}

	void SimpleRenderer::Init()
	{
		m_Shader = CreateRef<Shader>("assets/shaders/test.shader");
		m_Pipeline = CreateRef<VulkanPipline>(m_Shader);

		InitCommandBuffers();
	}

	void SimpleRenderer::InitCommandBuffers()
	{
		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();
		Ref<VulkanSwapChain> swapChain = Application::GetApp().GetVulkanSwapChain();
		QueueFamilyIndices queueIndices = device->GetQueueIndices();

		// Create command pool
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueIndices.GraphicsQueue.value();
		poolInfo.flags = 0; // Optional

		VkResult result = vkCreateCommandPool(device->GetLogicalDevice(), &poolInfo, nullptr, &m_CommandPool);
		ASSERT(result == VK_SUCCESS, "Failed to initialize Vulkan command pool");

		m_CommandBuffers.resize(swapChain->GetFramebuffers().size());

		// Create command buffers
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		result = vkAllocateCommandBuffers(device->GetLogicalDevice(), &allocInfo, m_CommandBuffers.data());
		ASSERT(result == VK_SUCCESS, "Failed to initialize Vulkan command buffers");

		for (size_t i = 0; i < m_CommandBuffers.size(); i++)
		{
			// Begin command buffer
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0; // Optional
			beginInfo.pInheritanceInfo = nullptr; // Optional

			result = vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo);
			ASSERT(result == VK_SUCCESS, "Failed to begin Vulkan command buffer");

			// Begin render pass
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = swapChain->GetRenderPass();
			renderPassInfo.framebuffer = swapChain->GetFramebuffers()[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChain->GetExtent();

			VkClearValue clearColor = { {{0.2f, 0.6f, 0.2f, 1.0f}} };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			
			// Record commands
			vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipeline());

			vkCmdDraw(m_CommandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(m_CommandBuffers[i]);

			// End command recording
			result = vkEndCommandBuffer(m_CommandBuffers[i]);
			ASSERT(result == VK_SUCCESS, "Failed to record Vulkan command buffer");
		}
	}

	void SimpleRenderer::Render()
	{
	}

}