#pragma once
#include "VulkanPlayground/Graphics/Shader.h"
#include "VulkanPlayground/Graphics/VulkanPipeline.h"
#include "VulkanPlayground/Graphics/VulkanBuffers.h"
#include "VulkanPlayground/Graphics/ImGUI/ImGUILayer.h"
#include "VulkanPlayground/Graphics/Texture.h"
#include "VulkanPlayground/Graphics/VulkanFramebuffer.h"

namespace VKPlayground  {

	class SimpleRenderer
	{
	public:
		SimpleRenderer();
		~SimpleRenderer();

	public:
		void BeginFrame();
		void EndFrame();

		void BeginRenderPass();
		void EndRenderPass();

		void Render();
		void RenderUI();

		void OnImGuiRender();

		static VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetAllocateInfo allocInfo);

	private:
		void Init();
		void CreateDescriptorPools();
		std::vector<VkDescriptorSet> AllocateDescriptorSets(const std::vector<VkDescriptorSetLayout>& layouts);

	private:
		Ref<Shader> m_Shader;
		Ref<VulkanPipeline> m_Pipeline;

		Ref<VulkanVertexBuffer> m_VertexBuffer;
		Ref<VulkanIndexBuffer> m_IndexBuffer;
		Ref<VulkanUniformBuffer> m_UniformBuffer;
		
		Ref<Texture2D> m_Texture;
		Ref<VulkanFramebuffer> m_Framebuffer;

		VkCommandBuffer m_ActiveCommandBuffer = nullptr;

		std::vector<VkDescriptorSet> m_DescriptorSets;
		std::vector<VkDescriptorPool> m_DescriptorPools;
	};

}