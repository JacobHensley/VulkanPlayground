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
	}

	void SimpleRenderer::Init()
	{
		m_Shader = CreateRef<Shader>("assets/shaders/test.shader");
		m_Pipeline = CreateRef<VulkanPipeline>(m_Shader);

		// Create vertex buffer
		glm::vec3 vertices[4] = {
			{ -0.5f, -0.5f, 0.0f },
			{  0.5f, -0.5f, 0.0f },
			{  0.5f,  0.5f, 0.0f },
			{ -0.5f,  0.5f, 0.0f }
		};

		m_VertexBuffer = CreateRef<VertexBuffer>(&vertices, sizeof(vertices));

		// Create index buffer
		uint16_t indices[6] = { 0, 1, 2, 2, 3, 0 };

		m_IndexBuffer = CreateRef<IndexBuffer>(&indices, sizeof(indices));

		RecordCommandBuffers();
	}

	void SimpleRenderer::RecordCommandBuffers()
	{
		Ref<VulkanSwapChain> swapChain = Application::GetApp().GetVulkanSwapChain();
		const std::vector<VkCommandBuffer>& commandBuffers = swapChain->GetCommandBuffers();

		for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			// Begin command buffer
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0; // Optional
			beginInfo.pInheritanceInfo = nullptr; // Optional

			VkResult result = vkBeginCommandBuffer(commandBuffers[i], &beginInfo);
			ASSERT(result == VK_SUCCESS, "Failed to begin Vulkan command buffer");

			// Set clear color
			VkClearValue clearColor = { {{0.1f, 0.1f, 0.8f, 1.0f}} };
			
			// Begin render pass
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = swapChain->GetRenderPass();
			renderPassInfo.framebuffer = swapChain->GetFramebuffers()[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChain->GetExtent();
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			// Record commands
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipeline());

			VkDeviceSize offset = 0;
			VkBuffer vertexBuffer = m_VertexBuffer->GetVulkanBuffer();
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer, &offset);

			vkCmdBindIndexBuffer(commandBuffers[i], m_IndexBuffer->GetVulkanBuffer(), 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(commandBuffers[i], 6, 1, 0, 0, 0);
			vkCmdEndRenderPass(commandBuffers[i]);

			// End command recording
			result = vkEndCommandBuffer(commandBuffers[i]);
			ASSERT(result == VK_SUCCESS, "Failed to record Vulkan command buffer");
		}
	}

	void SimpleRenderer::RecreateSwapChain()
	{
		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();
		vkDeviceWaitIdle(device->GetLogicalDevice());

		Ref<VulkanSwapChain> swapChain = Application::GetApp().GetVulkanSwapChain();

		swapChain.reset();
		m_Pipeline.reset();
		
		swapChain = CreateRef<VulkanSwapChain>();
		m_Pipeline = CreateRef<VulkanPipeline>(m_Shader);
	}

	// TODO: Comment this function
	void SimpleRenderer::Render()
	{
		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();
		Ref<VulkanSwapChain> swapChain = Application::GetApp().GetVulkanSwapChain();
		
		std::vector<VkCommandBuffer>& commandBuffers = swapChain->GetCommandBuffers();

		std::vector<VkSemaphore>& imageAvailableSemaphores = swapChain->GetImageAvailableSemaphores();
		std::vector<VkSemaphore>& renderFinishedSemaphores = swapChain->GetRenderFinishedSemaphores();

		std::vector<VkFence>& inFlightFences = swapChain->GetInFlightFences();
		std::vector<VkFence>& imagesInFlight = swapChain->GetImagesInFlight();

		vkWaitForFences(device->GetLogicalDevice(), 1, &inFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
		vkResetFences(device->GetLogicalDevice(), 1, &inFlightFences[m_CurrentFrame]);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(device->GetLogicalDevice(), swapChain->GetSwapChainHandle(), UINT64_MAX, imageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			ASSERT(false, "HELLO");
		}

		if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(device->GetLogicalDevice(), 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		imagesInFlight[imageIndex] = inFlightFences[m_CurrentFrame];

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[m_CurrentFrame] };
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device->GetLogicalDevice(), 1, &inFlightFences[m_CurrentFrame]);

		result = vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, inFlightFences[m_CurrentFrame]);
	//	ASSERT(result == VK_SUCCESS, "Failed to submit Vulkan queue");

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			ASSERT(false, "HELLO");
		}

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

		m_CurrentFrame = (m_CurrentFrame + 1) % 2;
	}

}