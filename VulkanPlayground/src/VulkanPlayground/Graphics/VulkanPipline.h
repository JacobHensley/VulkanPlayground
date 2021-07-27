#pragma once
#include "VulkanPlayground/Graphics/Shader.h"
#include <vulkan/vulkan.h>

namespace VKPlayground {

	class VulkanPipline
	{
	public:
		VulkanPipline(Ref<Shader> shader);
		~VulkanPipline();

	private:
		void Init();

	private:
		VkPipeline m_Pipeline;
		VkPipelineLayout m_PipelineLayout;

		Ref<Shader> m_Shader;
	};

}