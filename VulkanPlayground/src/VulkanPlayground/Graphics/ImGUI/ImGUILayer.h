#pragma once
#include <Vulkan/vulkan.h>

namespace VKPlayground {

	class ImGUILayer
	{
	public: 
		ImGUILayer();
		~ImGUILayer();

		void BeginFrame();
		void EndFrame();

	private:
		void Init();

	private:
		VkDescriptorPool m_DescriptorPool;
	};

}