#include "pch.h"
#include "SimpleRenderer.h"
#include "Application.h"
#include "VulkanPlayground/Graphics/VulkanDevice.h"
#include "VulkanPlayground/Core/VulkanTools.h"
#include <vulkan/vulkan.h>

namespace VKPlayground {

	struct ColorBuffer
	{
		glm::vec3 Color;
	};

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

		m_VertexBuffer = CreateRef<VulkanVertexBuffer>(&vertices, sizeof(vertices));

		// Create index buffer
		uint16_t indices[6] = { 0, 1, 2, 2, 3, 0 };

		m_IndexBuffer = CreateRef<VulkanIndexBuffer>(&indices, sizeof(indices));

		// Create uniform buffer
		ColorBuffer colorBuffer;
		colorBuffer.Color = glm::vec3(1.0f, 0.2f, 0.2f);

		m_UniformBuffer = CreateRef<VulkanUniformBuffer>(&colorBuffer, sizeof(colorBuffer));

		CreateDescriptorPools();
	}

	void SimpleRenderer::CreateDescriptorPools()
	{
		Ref<VulkanSwapChain> swapChain = Application::GetApp().GetVulkanSwapChain();
		VkDevice device = Application::GetApp().GetVulkanDevice()->GetLogicalDevice();

		// Create descriptor pool for each frame in flight
		m_DescriptorPools.resize(swapChain->GetFramesInFlight());

		// Define max number of each descriptor for each descriptor set
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 }
		};

		// Create descriptor pool
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.flags = 0; // VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		descriptorPoolCreateInfo.maxSets = 1000;
		descriptorPoolCreateInfo.poolSizeCount = 2;
		descriptorPoolCreateInfo.pPoolSizes = poolSizes;

		for (auto& descriptorPool : m_DescriptorPools)
		{
			VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool));
		}
	}

	void SimpleRenderer::BeginFrame()
	{
		Ref<VulkanSwapChain> swapChain = Application::GetApp().GetVulkanSwapChain();
		VkDevice device = Application::GetApp().GetVulkanDevice()->GetLogicalDevice();
		uint32_t frameIndex = swapChain->GetCurrentBufferIndex();

		// Reset descriptor pool for current frame about to begin
		VK_CHECK_RESULT(vkResetDescriptorPool(device, m_DescriptorPools[frameIndex], 0));

		// Allocate all descriptor sets
		const std::vector<VkDescriptorSetLayout>& layouts = m_Shader->GetDescriptorSetLayouts();
		m_DescriptorSets = AllocateDescriptorSet(m_Shader->GetDescriptorSetLayouts());

		UniformBufferDescription uniformBuffer = m_Shader->GetUniformBufferDescriptions()[0];

		// Update correct descriptor set with data from uniform buffer 0
		VkWriteDescriptorSet uniformBufferWriteDescriptor = {};
		uniformBufferWriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		uniformBufferWriteDescriptor.descriptorCount = 1;
		uniformBufferWriteDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferWriteDescriptor.dstSet = m_DescriptorSets[uniformBuffer.Index];
		uniformBufferWriteDescriptor.dstBinding = uniformBuffer.BindingPoint;
		uniformBufferWriteDescriptor.pBufferInfo = &m_UniformBuffer->getDescriptorBufferInfo();
		uniformBufferWriteDescriptor.pImageInfo = nullptr;

		vkUpdateDescriptorSets(device, 1, &uniformBufferWriteDescriptor, 0, nullptr);
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

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, m_DescriptorSets.data(), 0, nullptr);

		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->GetVulkanBuffer(), 0, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffer);

		// End command recording
		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
	}

	std::vector<VkDescriptorSet> SimpleRenderer::AllocateDescriptorSet(const std::vector<VkDescriptorSetLayout>& layouts)
	{
		Ref<VulkanSwapChain> swapChain = Application::GetApp().GetVulkanSwapChain();
		VkDevice device = Application::GetApp().GetVulkanDevice()->GetLogicalDevice();
		uint32_t frameIndex = swapChain->GetCurrentBufferIndex();

		// Allocate descriptor sets from descriptor pool for current frame given the layout info
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorSetCount = layouts.size();
		allocInfo.pSetLayouts = layouts.data();
		allocInfo.descriptorPool = m_DescriptorPools[frameIndex];

		std::vector<VkDescriptorSet> result;
		result.resize(layouts.size());

		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, result.data()));
		return result;
	}

}