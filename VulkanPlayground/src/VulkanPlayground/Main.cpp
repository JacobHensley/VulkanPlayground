#include "pch.h"
#include "Core/Application.h"
#include "Graphics/API/VulkanDevice.h"

using namespace VKPlayground;

int main()
{
	Application application = Application("Vulkan Playground");

	VulkanDevice device = VulkanDevice();

	application.Run();

	return 0;
}