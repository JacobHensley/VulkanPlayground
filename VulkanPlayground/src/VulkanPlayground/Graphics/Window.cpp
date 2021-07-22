#include "pch.h"
#include "Window.h"

namespace VKPlayground {

    Window::Window(const std::string& name, int width, int height)
	    : m_Name(name), m_Width(width), m_Height(height)
    {
        Init();
        LOG_INFO("initialized glfw window");
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

    bool Window::IsClosed()
    {
        return glfwWindowShouldClose(m_WindowHandle);
    }

}