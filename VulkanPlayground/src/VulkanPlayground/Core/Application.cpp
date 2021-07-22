#include "pch.h"
#include "Application.h"

namespace VKPlayground {

	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string name)
		: m_Name(name)
	{
		Init();
		LOG_INFO("Initialized Application");
	}

	void Application::Init()
	{
		ASSERT(!s_Instance, "Instance of Application already exists");
		s_Instance = this;

		Log::Init();

		m_Window = CreateRef<Window>(m_Name, 1280, 720);
		m_VulkanInstance = CreateRef<VulkanInstance>(m_Name);
	}

	void Application::Run()
	{
		while (!m_Window->IsClosed())
		{	
			m_Window->Update();
		}
	}

}