#pragma once
#include "VulkanPlayground/Graphics/VulkanImage.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace VKPlayground {

	struct FramebufferSpecification
	{
		uint32_t Width = 0;
		uint32_t Height = 0;
		float Scale = 1.0f;
		std::vector<VkFormat> AttachmentFormats;
		glm::vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	};

	struct FramebufferAttachment
	{
		Ref<VulkanImage> Image;
		VkAttachmentDescription Description;
	};

	class VulkanFramebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferSpecification& specification);
		~VulkanFramebuffer();

	public:
		void Resize(uint32_t width, uint32_t height);
		void Invalidate();

		inline VkFramebuffer GetFramebuffer() { return m_Framebuffer; }
		inline VkRenderPass GetRenderPass() { return m_RenderPass; }

		inline uint32_t GetWidth() { return m_Width; }
		inline uint32_t GetHeight() { return m_Height; }

	private:
		void Init();

	private:
		FramebufferSpecification m_Specification;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;

		VkFramebuffer m_Framebuffer = nullptr;
		VkRenderPass m_RenderPass = nullptr;

		std::vector<FramebufferAttachment> m_Attachments;
	};

}