#pragma once
#include "vulkan/vulkan.h"

namespace VKPlayground {

	class VulkanDevice
	{
	public:
		VulkanDevice();
		~VulkanDevice();

	private:
		void Init();

	private:
		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_LogicalDevice;

		VkQueue m_GraphicsQueueHandle;
	};

}