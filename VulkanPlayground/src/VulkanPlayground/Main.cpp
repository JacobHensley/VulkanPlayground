#include "pch.h"
#include "Core/Application.h"
#include "VulkanPlayground/Graphics/Shader.h"

using namespace VKPlayground;

int main()
{
	Application application = Application("Vulkan Playground");

	Shader shader("assets/shaders/test.shader");

	application.Run();

	return 0;
}