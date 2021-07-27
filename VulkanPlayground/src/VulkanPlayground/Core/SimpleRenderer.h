#pragma once
#include "VulkanPlayground/Graphics/Shader.h"
#include "VulkanPlayground/Graphics/VulkanPipline.h"
#include <vulkan/vulkan.h>

namespace VKPlayground  {

	class SimpleRenderer
	{
	public:
		SimpleRenderer();
		~SimpleRenderer();

	public:
		void Render();

	private:
		void Init();
		void InitCommandBuffers();

	private:
		Ref<Shader> m_Shader;
		Ref<VulkanPipline> m_Pipeline;

		VkCommandPool m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;
	};

}