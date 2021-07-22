#pragma once
#include "vulkan/vulkan.h"

class VulkanInstance
{
public:
	VulkanInstance();
	~VulkanInstance();

private:
	void Init();
	void InitDebugCallback();

private:
	VkInstance m_Instance;

	PFN_vkCreateDebugUtilsMessengerEXT m_CreateDebugUtilsMessengerEXT;
	PFN_vkDestroyDebugUtilsMessengerEXT m_DestroyDebugUtilsMessengerEXT;
	VkDebugUtilsMessengerEXT m_DebugUtilsMessenger;
};