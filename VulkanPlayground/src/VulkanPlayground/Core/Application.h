#pragma once
#include "VulkanPlayground/Graphics/Window.h"
#include "VulkanPlayground/Graphics/VulkanInstance.h"
#include "VulkanPlayground/Graphics/VulkanDevice.h"
#include "VulkanPlayground/Graphics/VulkanSwapChain.h"

namespace VKPlayground {

	class Application
	{
	public:
		Application(const std::string name);
		~Application();

	public:
		void Run();

		inline Ref<Window> GetWindow() { return m_Window; }
		inline Ref<VulkanInstance> GetVulkanInstance() { return m_VulkanInstance; }
		inline Ref<VulkanDevice> GetVulkanDevice() { return m_Device; }

		inline static Application& GetApp() { return *s_Instance; }

	private:
		void Init();

	private:
		std::string m_Name;
		Ref<Window> m_Window;

		// TODO: Move to context class
		Ref<VulkanInstance> m_VulkanInstance;
		Ref<VulkanDevice> m_Device;
		Ref<VulkanSwapChain> m_SwapChain;

	private:
		static Application* s_Instance;
	};

}