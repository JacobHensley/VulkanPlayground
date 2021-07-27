#pragma once
#include <vulkan/vulkan.h>

namespace VKPlayground {

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> GraphicsQueue;
		std::optional<uint32_t> PresentQueue;

		bool isComplete()
		{
			return GraphicsQueue.has_value() && PresentQueue.has_value();
		}
	};

	struct SwapChainSupportDetails 
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	class VulkanDevice
	{
	public:
		VulkanDevice();
		~VulkanDevice();

	public:
		inline SwapChainSupportDetails GetSwapChainSupportDetails() { return m_SwapChainSupportDetails; }
		inline QueueFamilyIndices GetQueueIndices() { return m_QueueIndices; }
		
		inline VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; };
		inline VkDevice GetLogicalDevice() { return m_LogicalDevice; };

	private:
		void Init();

		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueIndices(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

	private:
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_LogicalDevice = VK_NULL_HANDLE;

		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;

		SwapChainSupportDetails m_SwapChainSupportDetails;
		QueueFamilyIndices m_QueueIndices;
	};

}