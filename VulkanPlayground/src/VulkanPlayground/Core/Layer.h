#pragma once

namespace VKPlayground {

	class Layer
	{
	public:
		Layer(const std::string& name);
		virtual ~Layer();

	public:
		virtual void OnAttach() {}
		virtual void OnDetach() {}

		virtual void Update() {}
		virtual void Render() {}

		virtual void ImGUIRender() {}

	protected:
		const std::string m_Name;
	};

}