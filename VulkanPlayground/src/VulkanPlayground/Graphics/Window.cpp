#include "pch.h"
#include "Window.h"
#include "VulkanPlayground/Core/Application.h"

namespace VKPlayground {

    Window::Window(const std::string& name, int width, int height)
	    : m_Name(name), m_Width(width), m_Height(height)
    {
        Init();
        LOG_INFO("Initialized glfw window");
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_WindowHandle);
        glfwTerminate();
    }

    void Window::Init()
    {
        bool initialized = glfwInit();
        ASSERT(initialized, "Failed to initialize glfw window");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_WindowHandle = glfwCreateWindow(m_Width, m_Height, m_Name.c_str(), nullptr, nullptr);
    }

    void Window::Update()
    {
        glfwPollEvents();
    }

    void Window::InitVulkanSurface()
    {
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = glfwGetWin32Window(m_WindowHandle);
        createInfo.hinstance = GetModuleHandle(nullptr);

        VkInstance instance = Application::GetApp().GetVulkanInstance()->GetInstanceHandle();

        VkResult result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &m_VulkanSurface);
        ASSERT(result == VK_SUCCESS, "Failed to initialize Vulkan surface");

        LOG_INFO("Initialized Vulkan surface");
    }

    glm::vec2 Window::GetFramebufferSize()
    {
        glm::ivec2 result;
        glfwGetFramebufferSize(m_WindowHandle, &result.x, &result.y);
        return result;
    }

    bool Window::IsClosed()
    {
        return glfwWindowShouldClose(m_WindowHandle);
    }

}