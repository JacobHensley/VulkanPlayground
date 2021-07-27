#include "pch.h"
#include "Core/Application.h"
#include "VulkanPlayground/Graphics/Shader.h"
#include "VulkanPlayground/Graphics/VulkanPipline.h"

using namespace VKPlayground;

int main()
{
	Application application = Application("Vulkan Playground");

	Ref<Shader> shader = CreateRef<Shader>("assets/shaders/test.shader");
	VulkanPipline pipeline = VulkanPipline(shader);

	application.Run();

	return 0;
}