#pragma once
#include "VulkanPlayground/Graphics/Shader.h"
#include "VulkanPlayground/Graphics/VulkanPipeline.h"
#include "VulkanPlayground/Graphics/VulkanBuffers.h"
#include "VulkanPlayground/Graphics/ImGUI/ImGUILayer.h"

namespace VKPlayground  {

	class SimpleRenderer
	{
	public:
		SimpleRenderer();
		~SimpleRenderer();

	public:
		void BeginFrame();
		void Render();

	private:
		void Init();
		void CreateDescriptorPools();
		std::vector<VkDescriptorSet> AllocateDescriptorSet(const std::vector<VkDescriptorSetLayout>& layouts);

	private:
		Ref<Shader> m_Shader;
		Ref<VulkanPipeline> m_Pipeline;

		Ref<VulkanVertexBuffer> m_VertexBuffer;
		Ref<VulkanIndexBuffer> m_IndexBuffer;
		Ref<VulkanUniformBuffer> m_UniformBuffer;

		std::vector<VkDescriptorSet> m_DescriptorSets;
		std::vector<VkDescriptorPool> m_DescriptorPools;
	};

}