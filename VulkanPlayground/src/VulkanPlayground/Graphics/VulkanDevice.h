#pragma once
#include <vulkan/vulkan.h>

namespace VKPlayground {

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> GraphicsQueue;
		std::optional<uint32_t> PresentQueue;
		std::optional<uint32_t> TransferQueue;

		bool isComplete()
		{
			return GraphicsQueue.has_value() && PresentQueue.has_value() && TransferQueue.has_value();
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
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

		inline QueueFamilyIndices GetQueueIndices() { return m_QueueIndices; }
		
		inline VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }
		inline VkQueue GetPresentsQueue() { return m_PresentQueue; }

		inline VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; };
		inline VkDevice GetLogicalDevice() { return m_LogicalDevice; };

	private:
		void Init();

		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueIndices(VkPhysicalDevice device);

	private:
		VkPhysicalDevice m_PhysicalDevice = nullptr;
		VkDevice m_LogicalDevice = nullptr;

		VkQueue m_GraphicsQueue = nullptr;
		VkQueue m_PresentQueue = nullptr;

		SwapChainSupportDetails m_SwapChainSupportDetails;
		QueueFamilyIndices m_QueueIndices;
	};

}