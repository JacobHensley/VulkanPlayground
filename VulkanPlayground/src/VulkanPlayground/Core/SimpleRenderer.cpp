#include "pch.h"
#include "SimpleRenderer.h"
#include "Application.h"
#include "VulkanPlayground/Graphics/ImGUI/imgui_impl_vulkan_with_textures.h"
#include "VulkanPlayground/Graphics/VulkanDevice.h"
#include "VulkanPlayground/Core/VulkanTools.h"
#include <vulkan/vulkan.h>
#include <glm/gtc/type_ptr.hpp>

namespace VKPlayground {

	struct CameraBuffer
	{
		glm::mat4 View;
		glm::mat4 Proj;
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

		m_Camera = CreateRef<Camera>(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 100.0f));
		m_Shader = CreateRef<Shader>("assets/shaders/test.shader");

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

		m_IndexBuffer = CreateRef<VulkanIndexBuffer>(&indices, sizeof(indices), 6);

		// Create uniform buffer
		CameraBuffer cameraBuffer;
		cameraBuffer.View = m_Camera->GetViewMatrix();
		cameraBuffer.Proj = m_Camera->GetProjectionMatrix();

		m_UniformBuffer = CreateRef<VulkanUniformBuffer>(&cameraBuffer, sizeof(cameraBuffer));

		CreateDescriptorPools();

		// Resources
		m_Texture = CreateRef<Texture2D>("assets/textures/ChernoLogo.png");
		m_Mesh = CreateRef<Mesh>("assets/models/Cube.gltf");

		FramebufferSpecification spec;
		spec.AttachmentFormats = { VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_D24_UNORM_S8_UINT };
		spec.Width = 1280;
		spec.Height = 720;
		m_Framebuffer = CreateRef<VulkanFramebuffer>(spec);

		m_Pipeline = CreateRef<VulkanPipeline>(m_Shader, m_Framebuffer->GetRenderPass());
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

		m_Camera->Update();

		// Create uniform buffer
		CameraBuffer cameraBuffer;
		cameraBuffer.View = m_Camera->GetViewMatrix() * glm::scale(glm::mat4(1.0f), glm::vec3(5.0));
		cameraBuffer.Proj = m_Camera->GetProjectionMatrix();

		// Create function to update buffer instead of creating the object again
		m_UniformBuffer->UpdateBuffer(&cameraBuffer);
	//	m_UniformBuffer = CreateRef<VulkanUniformBuffer>(&cameraBuffer, sizeof(cameraBuffer));

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

	void SimpleRenderer::BeginRenderPass(Ref<VulkanFramebuffer> framebuffer)
	{
		VkRenderPass renderPass;
		VkFramebuffer vulkanFramebuffer;
		VkExtent2D extent;

		if (framebuffer)
		{
			vulkanFramebuffer = framebuffer->GetFramebuffer();
			renderPass = framebuffer->GetRenderPass();
			extent = { framebuffer->GetWidth(), framebuffer->GetHeight() };
		}
		else
		{
			// Use swapchain if no framebuffer
			Ref<VulkanSwapChain> swapChain = Application::GetApp().GetVulkanSwapChain();
			vulkanFramebuffer = swapChain->GetCurrentFramebuffer();
			renderPass = swapChain->GetRenderPass();
			extent = swapChain->GetExtent();
		}


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
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = vulkanFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;

		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = clearColor;
		if (framebuffer)
		{
			renderPassInfo.clearValueCount = 2;

		}

		// Update viewport
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)extent.width;
		viewport.height = (float)extent.height;
		if (!framebuffer)
		{
			viewport.y = (float)extent.height;
			viewport.height = -(float)extent.height;
		}
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
		VkBuffer vertexBuffer = m_Mesh->GetVertexBuffer()->GetVulkanBuffer();
		vkCmdBindVertexBuffers(m_ActiveCommandBuffer, 0, 1, &vertexBuffer, &offset);
		vkCmdBindIndexBuffer(m_ActiveCommandBuffer, m_Mesh->GetIndexBuffer()->GetVulkanBuffer(), 0, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(m_ActiveCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, m_DescriptorSets.data(), 0, nullptr);
		vkCmdDrawIndexed(m_ActiveCommandBuffer, m_Mesh->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

	void SimpleRenderer::RenderUI()
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_ActiveCommandBuffer);
	}

	void SimpleRenderer::OnImGuiRender()
	{
	//	ImGui::Begin("Example");
	//	ImGui::Button("Hello");

	//	auto& descriptorInfo = m_Texture->GetDescriptorImageInfo();
	//	ImTextureID imTex = ImGui_ImplVulkan_AddTexture(descriptorInfo.sampler, descriptorInfo.imageView, descriptorInfo.imageLayout);
	//	ImGui::Image(imTex, { 512, 512 }, ImVec2(0, 1), ImVec2(1, 0));
	//	ImGui::End();

		// Viewport
		{
			ImGui::Begin("Viewport");

			auto& descriptorInfo = m_Framebuffer->GetImage(0)->GetDescriptorImageInfo();
			ImTextureID imTex = ImGui_ImplVulkan_AddTexture(descriptorInfo.sampler, descriptorInfo.imageView, descriptorInfo.imageLayout);

			const auto& fbSpec = m_Framebuffer->GetSpecification();
			float width = ImGui::GetContentRegionAvail().x;
			float aspect = (float)fbSpec.Height / (float)fbSpec.Width;

			ImGui::Image(imTex, { width, width * aspect }, ImVec2(0, 1), ImVec2(1, 0));
			ImGui::End();
		}
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