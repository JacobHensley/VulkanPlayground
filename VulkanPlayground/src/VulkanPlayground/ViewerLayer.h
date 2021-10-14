#pragma once
#include "VulkanPlayground/Core/Layer.h"
#include "VulkanPlayground/Graphics/Camera.h"
#include "VulkanPlayground/Graphics/Mesh.h"
#include <glm/gtc/type_ptr.hpp>

namespace VKPlayground {

	class ViewerLayer : public Layer
	{
	public:
		ViewerLayer();

	public:
		void Init();

		void Update();
		void Render();
		void ImGUIRender();

	private:
		Ref<Camera> m_Camera;
		Ref<Mesh> m_Mesh;
		glm::mat4 m_MeshTransform;
	};

}