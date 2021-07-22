#include "pch.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanPlayground/Core/Application.h"

namespace VKPlayground {

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsQueue;
		std::optional<uint32_t> computeQueue;
		std::optional<uint32_t> transferQueue;
	};

	VulkanDevice::VulkanDevice()
	{
		Init();
		LOG_INFO("Initialized device");
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
		
		// TODO: Check for more requirements in the future such as geometry shaders

		// Select first device if there is no dedicated GPU
		m_PhysicalDevice = devices[0];

		// Select first dedicated GPU
		for (int i = 0; i < deviceCount; i++)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);

			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(devices[i], &deviceFeatures);

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				m_PhysicalDevice = devices[i];
				LOG_INFO("Selected GPU: {0}", deviceProperties.deviceName);
			}
		}

		// Get queue family info
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

		// Set queue family indices 
		// TODO: Do more sanity checks here and set compute and transfer queue indices to grahpics queue index if they do not exist
		QueueFamilyIndices indices;
		for (int i = 0; i < queueFamilyCount; i++)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) 
			{
				indices.graphicsQueue = i;
			}

			if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				indices.computeQueue = i;
			}

			if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				indices.transferQueue = i;
			}
		}

		// Graphics queue info
		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo graphicsQueueCreateInfo{};
		graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		graphicsQueueCreateInfo.queueFamilyIndex = indices.graphicsQueue.value();
		graphicsQueueCreateInfo.queueCount = 1;
		graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;

		// Required device features
		VkPhysicalDeviceFeatures deviceFeatures{};

		// Logical device info
		// TODO: Add info about device specific extensions here in the future to support older verions of Vulkan
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &graphicsQueueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;

		// Create logical device
		VkResult result = vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_LogicalDevice);
		ASSERT(result == VK_SUCCESS, "Failed to initialize logical device");

		// Create queue handles
		// TODO: Create queue handles for more types of available queues
		vkGetDeviceQueue(m_LogicalDevice, indices.graphicsQueue.value(), 0, &m_GraphicsQueueHandle);
	}

}