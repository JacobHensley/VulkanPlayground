#include "pch.h"
#include "Application.h"
#include "VulkanPlayground/Graphics/VulkanAllocator.h"
#include <imgui.h>

namespace VKPlayground {

	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string name)
		: m_Name(name)
	{
		Init();
		LOG_INFO("Initialized Application");
	}

	Application::~Application()
	{
		m_Renderer.reset();
		m_ImGUILayer.reset();
		m_SwapChain.reset();
		VulkanAllocator::Shutdown();
		m_Device.reset();
		m_Window.reset();
		m_VulkanInstance.reset();
	}

	void Application::Init()
	{
		ASSERT(!s_Instance, "Instance of Application already exists");
		s_Instance = this;

		Log::Init();
		
		m_Window = CreateRef<Window>(m_Name, 1280, 720);
		m_VulkanInstance = CreateRef<VulkanInstance>(m_Name);

		m_Window->InitVulkanSurface();

		m_Device = CreateRef<VulkanDevice>();
		m_SwapChain = CreateRef<VulkanSwapChain>();

		VulkanAllocator::Init(m_Device);

		m_Renderer = CreateRef<SimpleRenderer>();

		m_ImGUILayer = CreateRef<ImGUILayer>();
	}

	void Application::Run()
	{
		while (!m_Window->IsClosed())
		{	
			m_Window->Update();
			m_SwapChain->BeginFrame();
			m_Renderer->BeginFrame();

			// OnUpdate function
			{
				// ImGui
				m_ImGUILayer->BeginFrame();
				m_Renderer->OnImGuiRender();
				m_ImGUILayer->EndFrame();
			}

			// OnRenderFunction
			{
				m_Renderer->BeginRenderPass(m_Renderer->GetFramebuffer());

				// Render
				m_Renderer->Render();

				m_Renderer->EndRenderPass();

				m_Renderer->BeginRenderPass();
				m_Renderer->RenderUI();
				m_Renderer->EndRenderPass();
			}

			m_Renderer->EndFrame();
			m_SwapChain->Present();
		}

		Ref<VulkanDevice> device = Application::GetApp().GetVulkanDevice();
		vkDeviceWaitIdle(device->GetLogicalDevice());
	}

}