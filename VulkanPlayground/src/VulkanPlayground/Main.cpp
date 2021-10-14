#include "pch.h"
#include "ViewerLayer.h"
#include "Core/Application.h"

using namespace VKPlayground;

int main()
{
	Application application = Application("Vulkan Playground");

	Ref<ViewerLayer> layer = CreateRef<ViewerLayer>();
	application.AddLayer(layer);

	application.Run();

	return 0;
}