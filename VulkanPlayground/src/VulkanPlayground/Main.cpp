#include "pch.h"
#include "Graphics/Window.h"
#include "Graphics/API/VulkanInstance.h"

int main()
{
	Log::Init();

	Window window = Window("Vulkan Playground", 1280, 720);
	VulkanInstance instance = VulkanInstance();

	while (!window.IsClosed())
	{
		window.Update();
	}

	return 0;
}