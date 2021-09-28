#include "pch.h"
#include "SimpleRenderer.h"
#include "Application.h"
#include "VulkanPlayground/Graphics/ImGUI/impl_vulkan_with_textures.h"
#include "VulkanPlayground/Graphics/VulkanDevice.h"
#include "VulkanPlayground/Core/VulkanTools.h"
#include <vulkan/vulkan.h>

#include "tinygltf/tiny_gltf.h"

namespace VKPlayground {

	struct ColorBuffer
	{
		glm::vec3 Color;
	};

	static SimpleRenderer* s_Instance = nullptr;

	SimpleRenderer::SimpleRenderer()
	{
		s_Instance = this;

		Init();
		LOG_INFO("Initialized renderer");
	}

	SimpleRenderer::~SimpleRenderer()
	{
		VkDevice device = Application::GetApp().GetVulkanDevice()->GetLogicalDevice();

		for (int i = 0;i < m_DescriptorPools.size(); i++)
		{
			vkDestroyDescriptorPool(device, m_DescriptorPools[i], nullptr);
		}
	}

	void SimpleRenderer::Init()
	{
		Ref<VulkanSwapChain> swapChain = Application::GetApp().GetVulkanSwapChain();

		m_Shader = CreateRef<Shader>("assets/shaders/test.shader");
		m_Pipeline = CreateRef<VulkanPipeline>(m_Shader, swapChain->GetRenderPass());

		// Create vertex buffer
		struct Vertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		Vertex vertices[4] = {
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f } },
			{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f } },
			{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f } },
			{ { -0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f } }
		};

		m_VertexBuffer = CreateRef<VulkanVertexBuffer>(&vertices, sizeof(vertices));

		// Create index buffer
		uint16_t indices[6] = { 0, 1, 2, 2, 3, 0 };

		m_IndexBuffer = CreateRef<VulkanIndexBuffer>(&indices, sizeof(indices));

		// Create uniform buffer
		ColorBuffer colorBuffer;
		colorBuffer.Color = glm::vec3(1.0f, 0.3f, 0.3f);

		m_UniformBuffer = CreateRef<VulkanUniformBuffer>(&colorBuffer, sizeof(colorBuffer));

		CreateDescriptorPools();

		// Resources
		m_Texture = CreateRef<Texture2D>("assets/textures/ChernoLogo.png");
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
		m_DescriptorSets = AllocateDescriptorSets(m_Shader->GetDescriptorSetLayouts());

		// Write descriptors
		UniformBufferDescription uniformBuffer = m_Shader->GetUniformBufferDescriptions()[0];
		ShaderResource texture = m_Shader->GetShaderResourceDescriptions()[0];

		// Update correct descriptor set with data from uniform buffer 0
		VkWriteDescriptorSet uniformBufferWriteDescriptor = {};
		uniformBufferWriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		uniformBufferWriteDescriptor.descriptorCount = 1;
		uniformBufferWriteDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferWriteDescriptor.dstSet = m_DescriptorSets[uniformBuffer.Index];
		uniformBufferWriteDescriptor.dstBinding = uniformBuffer.BindingPoint;
		uniformBufferWriteDescriptor.pBufferInfo = &m_UniformBuffer->getDescriptorBufferInfo();
		uniformBufferWriteDescriptor.pImageInfo = nullptr;

		// Update correct descriptor set with data from texture 0
		VkWriteDescriptorSet textureWriteDescriptor = {};
		textureWriteDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		textureWriteDescriptor.descriptorCount = 1;
		textureWriteDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureWriteDescriptor.dstSet = m_DescriptorSets[texture.Index];
		textureWriteDescriptor.dstBinding = texture.BindingPoint;
		textureWriteDescriptor.pBufferInfo = nullptr;
		textureWriteDescriptor.pImageInfo = &m_Texture->GetDescriptorImageInfo();

		VkWriteDescriptorSet writeDescriptors[2] = { uniformBufferWriteDescriptor, textureWriteDescriptor };

		vkUpdateDescriptorSets(device, 2, writeDescriptors, 0, nullptr);

		// Start command buffer
		m_ActiveCommandBuffer = swapChain->GetCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;                  // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		VK_CHECK_RESULT(vkBeginCommandBuffer(m_ActiveCommandBuffer, &beginInfo));
	}

	void SimpleRenderer::EndFrame()
	{
		// End command recording
		VK_CHECK_RESULT(vkEndCommandBuffer(m_ActiveCommandBuffer));
	}

	void SimpleRenderer::BeginRenderPass()
	{
		Ref<VulkanSwapChain> swapChain = Application::GetApp().GetVulkanSwapChain();

		// Set clear color
		static float r = 0.0f;
		static float dir = 0.001f;
		r += dir;
		if (r > 1.0f)
			dir *= -1.0f;
		if (r < 0.0f)
			dir *= -1.0f;

		VkClearValue clearColor[2];
		clearColor[0].color = { r, 0.1f, 0.8f, 1.0f };
		clearColor[1].depthStencil = { 1.0f, 0 };

		// Begin render pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = swapChain->GetRenderPass();
		renderPassInfo.framebuffer = swapChain->GetCurrentFramebuffer();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain->GetExtent();
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = clearColor;

		// Update viewport
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = (float)swapChain->GetExtent().height;
		viewport.width = (float)swapChain->GetExtent().width;
		viewport.height = -(float)swapChain->GetExtent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_ActiveCommandBuffer, 0, 1, &viewport);

		vkCmdBeginRenderPass(m_ActiveCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void SimpleRenderer::EndRenderPass()
	{
		vkCmdEndRenderPass(m_ActiveCommandBuffer);
	}

	void SimpleRenderer::Render()
	{
		Ref<VulkanSwapChain> swapChain = Application::GetApp().GetVulkanSwapChain();

		vkCmdBindPipeline(m_ActiveCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipeline());

		VkDeviceSize offset = 0;
		VkBuffer vertexBuffer = m_VertexBuffer->GetVulkanBuffer();
		vkCmdBindVertexBuffers(m_ActiveCommandBuffer, 0, 1, &vertexBuffer, &offset);
		vkCmdBindIndexBuffer(m_ActiveCommandBuffer, m_IndexBuffer->GetVulkanBuffer(), 0, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(m_ActiveCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, m_DescriptorSets.data(), 0, nullptr);
		vkCmdDrawIndexed(m_ActiveCommandBuffer, 6, 1, 0, 0, 0);
	}

	void SimpleRenderer::RenderUI()
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_ActiveCommandBuffer);
	}

	void SimpleRenderer::OnImGuiRender()
	{
		ImGui::Begin("Example");
		ImGui::Button("Hello");

		auto& descriptorInfo = m_Texture->GetDescriptorImageInfo();
		ImTextureID imTex = ImGui_ImplVulkan_AddTexture(descriptorInfo.sampler, descriptorInfo.imageView, descriptorInfo.imageLayout);
		ImGui::Image(imTex, { 512, 512 }, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
	}

	std::vector<VkDescriptorSet> SimpleRenderer::AllocateDescriptorSets(const std::vector<VkDescriptorSetLayout>& layouts)
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

	VkDescriptorSet SimpleRenderer::AllocateDescriptorSet(VkDescriptorSetAllocateInfo allocInfo)
	{
		Ref<VulkanSwapChain> swapChain = Application::GetApp().GetVulkanSwapChain();
		VkDevice device = Application::GetApp().GetVulkanDevice()->GetLogicalDevice();
		uint32_t frameIndex = swapChain->GetCurrentBufferIndex();

		allocInfo.descriptorPool = s_Instance->m_DescriptorPools[frameIndex];

		VkDescriptorSet result;
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &result));
		return result;
	}

}