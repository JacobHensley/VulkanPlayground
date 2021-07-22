#pragma once
#include "GLFW/glfw3.h"

class Window
{
public:
	Window(const std::string& name, int width, int height);
	~Window();

public:
	void Update();
	bool IsClosed();

private:
	void Init();

private:
	const std::string m_Name;
	int m_Width;
	int m_Height;

	GLFWwindow* m_WindowHandle;
};