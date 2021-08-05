#pragma once
#include "VulkanPlayground/Core/Core.h"
#include "VulkanPlayground/Graphics/Shader.h"
#include <vulkan/vulkan.h>

namespace VKPlayground {

	class VulkanPipeline
	{
	public:
		VulkanPipeline(Ref<Shader> shader);
		~VulkanPipeline();

	public:
		inline VkPipeline GetPipeline() { return m_Pipeline; }

	private:
		void Init();

	private:
		VkPipeline m_Pipeline = nullptr;
		VkPipelineLayout m_PipelineLayout = nullptr;

		Ref<Shader> m_Shader;
	};

}