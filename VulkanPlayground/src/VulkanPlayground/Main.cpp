#include "pch.h"
#include "Core/Application.h"
#include "Core/SimpleRenderer.h"
#include "VulkanPlayground/Graphics/Shader.h"
#include "VulkanPlayground/Graphics/VulkanPipline.h"

using namespace VKPlayground;

int main()
{
	Application application = Application("Vulkan Playground");
	application.Run();

	return 0;
}