#include "pch.h"
#include "SimpleRenderer.h"
#include "Application.h"
#include "VulkanPlayground/Graphics/VulkanDevice.h"

namespace VKPlayground {

	static const int MAX_FRAMES_IN_FLIGHT = 2;

	SimpleRenderer::SimpleRenderer()
	{
		Init();
		LOG_INFO("Initialized renderer");
	}

	SimpleRenderer::~SimpleRenderer()
	{
		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();
		vkDestroyCommandPool(device->GetLogicalDevice(), m_CommandPool, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(device->GetLogicalDevice(), m_RenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device->GetLogicalDevice(), m_ImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device->GetLogicalDevice(), m_InFlightFences[i], nullptr);
		}
	}

	void SimpleRenderer::Init()
	{
		m_Shader = CreateRef<Shader>("assets/shaders/test.shader");
		m_Pipeline = CreateRef<VulkanPipeline>(m_Shader);

		// Create vertex buffer
		glm::vec3 vertices[3] = {
			{ -0.5f, -0.5f, 0.0f },
			{  0.0f,  0.5f, 0.0f },
			{  0.5f, -0.5f, 0.0f }
		};

		m_VertexBuffer = CreateRef<VertexBuffer>(&vertices, sizeof(vertices));

		InitCommandBuffers();

		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();
		Ref<VulkanSwapChain> swapChain = Application::GetApp().GetVulkanSwapChain();

		// Create synchronization objects
		m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_ImagesInFlight.resize(swapChain->GetFramebuffers().size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
		{
			VkResult result = vkCreateSemaphore(device->GetLogicalDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]);
			result = vkCreateSemaphore(device->GetLogicalDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]);
			result = vkCreateFence(device->GetLogicalDevice(), &fenceInfo, nullptr, &m_InFlightFences[i] );

			ASSERT(result == VK_SUCCESS, "Failed to initialize Vulkan Semaphores");
		}
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

			// Record commands
			vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			
			vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipeline());

			VkDeviceSize offset = 0;
			VkBuffer vertexBuffer = m_VertexBuffer->GetVulkanBuffer();
			vkCmdBindVertexBuffers(m_CommandBuffers[i], 0, 1, &vertexBuffer, &offset);

			vkCmdDraw(m_CommandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(m_CommandBuffers[i]);

			// End command recording
			result = vkEndCommandBuffer(m_CommandBuffers[i]);
			ASSERT(result == VK_SUCCESS, "Failed to record Vulkan command buffer");
		}
	}

	void SimpleRenderer::Render()
	{
		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();
		Ref<VulkanSwapChain> swapChain = Application::GetApp().GetVulkanSwapChain();
		
		vkWaitForFences(device->GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
		vkResetFences(device->GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

		uint32_t imageIndex;
		vkAcquireNextImageKHR(device->GetLogicalDevice(), swapChain->GetSwapChainHandle(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

		if (m_ImagesInFlight[imageIndex] != VK_NULL_HANDLE) 
		{
			vkWaitForFences(device->GetLogicalDevice(), 1, &m_ImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		m_ImagesInFlight[imageIndex] = m_InFlightFences[m_CurrentFrame];

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device->GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

		VkResult result = vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);
		ASSERT(result == VK_SUCCESS, "Failed to submit Vulkan queue");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain->GetSwapChainHandle() };

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		vkQueuePresentKHR(device->GetPresentsQueue(), &presentInfo);

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

}