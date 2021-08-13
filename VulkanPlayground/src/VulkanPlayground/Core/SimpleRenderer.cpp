#include "pch.h"
#include "SimpleRenderer.h"
#include "Application.h"
#include "VulkanPlayground/Graphics/VulkanDevice.h"
#include "VulkanPlayground/Core/VulkanTools.h"
#include <vulkan/vulkan.h>

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
	}

	void SimpleRenderer::Render()
	{
		Ref<VulkanSwapChain> swapChain = Application::GetApp().GetVulkanSwapChain();

		VkCommandBuffer commandBuffer = swapChain->GetCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;                  // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

		// Set clear color
		static float r = 0.0f;
		static float dir = 0.001f;
		r += dir;
		if (r > 1.0f)
			dir *= -1.0f;
		if (r < 0.0f)
			dir *= -1.0f;
		VkClearValue clearColor = { {{r, 0.1f, 0.8f, 1.0f}} };

		// Begin render pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = swapChain->GetRenderPass();
		renderPassInfo.framebuffer = swapChain->GetCurrentFramebuffer();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain->GetExtent();
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		// Record commands
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipeline());

		// Update viewport
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = (float)swapChain->GetExtent().height;
		viewport.width = (float)swapChain->GetExtent().width;
		viewport.height = -(float)swapChain->GetExtent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkDeviceSize offset = 0;
		VkBuffer vertexBuffer = m_VertexBuffer->GetVulkanBuffer();
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);

		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->GetVulkanBuffer(), 0, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffer);

		// End command recording
		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
	}

}