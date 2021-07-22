#include "pch.h"
#include "Core/Application.h"
#include "Graphics/API/VulkanDevice.h"
#include "Graphics/API/VulkanSwapChain.h"

using namespace VKPlayground;

int main()
{
	Application application = Application("Vulkan Playground");

	VulkanDevice device;
	VulkanSwapChain swapChain(device);

	application.Run();

	return 0;
}