#pragma once
#include "VulkanPlayground/Graphics/Window.h"
#include "VulkanPlayground/Graphics/API/VulkanInstance.h"

namespace VKPlayground {

	class Application
	{
	public:
		Application(const std::string name);

	public:
		void Run();

		inline Ref<Window> GetWindow() { return m_Window; }
		inline Ref<VulkanInstance> GetVulkanInstance() { return m_VulkanInstance; }

		inline static Application& GetApp() { return *s_Instance; }

	private:
		void Init();

	private:
		Ref<Window> m_Window;
		Ref<VulkanInstance> m_VulkanInstance;

		std::string m_Name;

	private:
		static Application* s_Instance;
	};

}