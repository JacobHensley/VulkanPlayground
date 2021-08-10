#pragma once
#include "VulkanPlayground/Graphics/Shader.h"
#include "VulkanPlayground/Graphics/VulkanPipeline.h"
#include "VulkanPlayground/Graphics/VertexBuffer.h"
#include "VulkanPlayground/Graphics/IndexBuffer.h"

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

	private:
		Ref<Shader> m_Shader;
		Ref<VulkanPipeline> m_Pipeline;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;

		size_t m_CurrentFrame = 0;
	};

}