#include "pch.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanPlayground/Core/Application.h"

static const std::vector<const char*> s_DeviceExtensions = 
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

namespace VKPlayground {

	VulkanDevice::VulkanDevice()
	{
		Init();
		LOG_INFO("Initialized Vulkan device");
	}

	VulkanDevice::~VulkanDevice()
	{
		vkDestroyDevice(m_LogicalDevice, nullptr);
	}

	void VulkanDevice::Init()
	{
		VkInstance instance = Application::GetApp().GetVulkanInstance()->GetInstanceHandle();

		// Get device info
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		ASSERT(deviceCount, "Could not find any devices that support Vulkan");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		// Loop though devices to find a suitable one
		QueueFamilyIndices indices;
		for (int i = 0; i < deviceCount; i++)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);

			indices = FindQueueIndices(devices[i]);

			if (IsDeviceSuitable(devices[i]))
			{
				m_SwapChainSupportDetails = QuerySwapChainSupport(devices[i]);
				m_QueueIndices = indices;
				m_PhysicalDevice = devices[i];
				LOG_INFO("Selected GPU: {0}", deviceProperties.deviceName);
			}
		}

		ASSERT(m_PhysicalDevice != VK_NULL_HANDLE, "Could not find any suitable device");

		// Create info for all queues
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.GraphicsQueue.value(), indices.PresentQueue.value(), indices.TransferQueue.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// Required device features
		VkPhysicalDeviceFeatures deviceFeatures{};

		// Logical device info
		// TODO: Add device specific extensions to support older verions of Vulkan
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(s_DeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = s_DeviceExtensions.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		// Create logical device
		VkResult result = vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_LogicalDevice);
		ASSERT(result == VK_SUCCESS, "Failed to initialize logical device");

		// Create queue handles
		vkGetDeviceQueue(m_LogicalDevice, indices.GraphicsQueue.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, indices.PresentQueue.value(), 0, &m_PresentQueue);
	}

	bool VulkanDevice::IsDeviceSuitable(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		QueueFamilyIndices indices = FindQueueIndices(device);
			
		// Check is all required extensions are supported
		bool extensionsSupported = CheckDeviceExtensionSupport(device);
		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			// Check to make sure there supported formats and present modes
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
		}

		// Check to make sure the device is dedicated and has all required queues
		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	bool VulkanDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		// Get extension info
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		// Convert s_DeviceExtensions to string set
		std::set<std::string> requiredExtensions(s_DeviceExtensions.begin(), s_DeviceExtensions.end());

		// Remove one extension from requiredExtensions for every one found
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices VulkanDevice::FindQueueIndices(VkPhysicalDevice device)
	{
		// Get queue family info
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		QueueFamilyIndices indices;
		for (int i = 0; i < queueFamilyCount; i++)
		{
			// Find graphics queue
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.GraphicsQueue = i;
			}

			// Find present queue
			// TODO: Pick best queue for presenting
			VkSurfaceKHR surface = Application::GetApp().GetWindow()->GetVulkanSurface();
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
			{
				indices.PresentQueue = i;
			}

			// Find transfer queue
			if ((queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
			{
				indices.TransferQueue = i;
			}
		}

		return indices;
	}

	SwapChainSupportDetails VulkanDevice::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		// Query surface capabilities
		VkSurfaceKHR surface = Application::GetApp().GetWindow()->GetVulkanSurface();
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);

		// Query formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.Formats.data());
		}

		// Query present modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.PresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentModes.data());
		}

		return details;
	}

}