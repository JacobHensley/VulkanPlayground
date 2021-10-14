#pragma once
#include "VulkanPlayground/Graphics/Camera.h"
#include "VulkanPlayground/Graphics/VulkanFramebuffer.h"
#include "VulkanPlayground/Graphics/VulkanPipeline.h"
#include "VulkanPlayground/Graphics/VulkanBuffers.h"
#include "VulkanPlayground/Graphics/Shader.h"
#include "VulkanPlayground/Graphics/Mesh.h"

namespace VKPlayground {
	
	struct CameraBuffer
	{
		glm::mat4 ViewProjection;
		glm::mat4 InverseViewProjection;
	};

	struct DrawCommand
	{
		SubMesh SubMesh;
		Ref<VulkanVertexBuffer> VertexBuffer;
		Ref<VulkanIndexBuffer> IndexBuffer;

		glm::mat4 Transform;
	};

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

	public:
		void BeginFrame();
		void EndFrame();

		void BeginScene(Ref<Camera> camera);
		void EndScene();

		void BeginRenderPass(Ref<VulkanFramebuffer> framebuffer = nullptr);
		void EndRenderPass();

		void SubmitMesh(Ref<Mesh> mesh, glm::mat4& transform);
		void Render();
		void RenderUI();

		void OnImGuiRender();

		Ref<VulkanFramebuffer> GetFramebuffer() { return m_Framebuffer; }

		static VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetAllocateInfo allocInfo);

	private:
		void Init();
		void CreateDescriptorPools();
		std::vector<VkDescriptorSet> AllocateDescriptorSets(const std::vector<VkDescriptorSetLayout>& layouts);

	private:
		Ref<Camera> m_ActiveCamera;

		CameraBuffer m_CameraBuffer;
		std::vector<DrawCommand> m_DrawList;
		Ref<VulkanUniformBuffer> m_CameraUniformBuffer;
		Ref<VulkanFramebuffer> m_Framebuffer;
		Ref<Shader> m_Shader;

		Ref<VulkanPipeline> m_Pipeline;
		VkCommandBuffer m_ActiveCommandBuffer = nullptr;
		std::vector<VkDescriptorSet> m_DescriptorSets;
		std::vector<VkDescriptorPool> m_DescriptorPools;
	};

}
