#pragma once
#include "VulkanPlayground/Graphics/ImGUI/ImGUILayer.h"
#include "VulkanPlayground/Graphics/Window.h"
#include "VulkanPlayground/Graphics/VulkanInstance.h"
#include "VulkanPlayground/Graphics/VulkanDevice.h"
#include "VulkanPlayground/Graphics/VulkanSwapChain.h"
#include "VulkanPlayground/Graphics/Renderer.h"
#include "VulkanPlayground/Core/Layer.h"

namespace VKPlayground {

	class Application
	{
	public:
		Application(const std::string name);
		~Application();

	public:
		void Run();

		inline void AddLayer(Ref<Layer> layer) { m_Layers.push_back(layer); }

		inline Ref<Window> GetWindow() { return m_Window; }
		inline Ref<VulkanInstance> GetVulkanInstance() { return m_VulkanInstance; }
		inline Ref<VulkanDevice> GetVulkanDevice() { return m_Device; }
		inline Ref<VulkanSwapChain> GetVulkanSwapChain() { return m_SwapChain; }
		inline Ref<Renderer> GetRenderer() { return m_Renderer; }

		inline static Application& GetApp() { return *s_Instance; }

	private:
		void Init();
		void Update();
		void Render();
		void ImGUIRender();

	private:
		std::string m_Name;
		Ref<Window> m_Window;

		Ref<ImGUILayer> m_ImGUILayer;

		// TODO: Move to context class
		Ref<VulkanInstance> m_VulkanInstance;
		Ref<VulkanDevice> m_Device;
		Ref<VulkanSwapChain> m_SwapChain;
		Ref<Renderer> m_Renderer;

		std::vector<Ref<Layer>> m_Layers;

	private:
		static Application* s_Instance;
	};

}