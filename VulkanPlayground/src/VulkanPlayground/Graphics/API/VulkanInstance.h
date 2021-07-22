#pragma once
#include "vulkan/vulkan.h"

namespace VKPlayground {

	class VulkanInstance
	{
	public:
		VulkanInstance(const std::string& name);
		~VulkanInstance();

	public:
		inline VkInstance GetInstanceHandle() { return m_Instance; }

	private:
		void Init();
		void InitDebugCallback();

	private:
		VkInstance m_Instance;
		std::string m_Name;

		PFN_vkCreateDebugUtilsMessengerEXT m_CreateDebugUtilsMessengerEXT;
		PFN_vkDestroyDebugUtilsMessengerEXT m_DestroyDebugUtilsMessengerEXT;
		VkDebugUtilsMessengerEXT m_DebugUtilsMessenger;
	};

}