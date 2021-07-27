#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>

namespace VKPlayground {

	class Window
	{
	public:
		Window(const std::string& name, int width, int height);
		~Window();

	public:
		void Update();

		void InitVulkanSurface();

		glm::vec2 GetFramebufferSize();
		bool IsClosed();

		inline VkSurfaceKHR GetVulkanSurface() { return m_VulkanSurface; }

	private:
		void Init();

	private:
		const std::string m_Name;
		int m_Width;
		int m_Height;

		GLFWwindow* m_WindowHandle;
		VkSurfaceKHR m_VulkanSurface;
	};

}
