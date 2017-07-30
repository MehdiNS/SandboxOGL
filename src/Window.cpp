#include "stdafx.h"
#include "Window.h"

namespace OGL
{
	Window::Window(u32 windowWidth, u32 windowHeight) :
		m_windowWidth{ windowWidth },
		m_windowHeight{ windowHeight },
		m_active{ true }
	{
		if (!glfwInit())
			exit(EXIT_FAILURE);

		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		m_glfwWindow = glfwCreateWindow(m_windowWidth, m_windowHeight, "OGL Sandbox", NULL, NULL);
	
		glfwMakeContextCurrent(m_glfwWindow);
		glfwIconifyWindow(m_glfwWindow);
		glfwRestoreWindow(m_glfwWindow);

		if (!m_glfwWindow)
		{
			glfwTerminate();
			exit(EXIT_FAILURE);
		}

		glfwMakeContextCurrent(m_glfwWindow);
		glfwSwapInterval(1); //V-sync

	}

	Window::~Window()
	{
		glfwDestroyWindow(m_glfwWindow);
		glfwTerminate();
	}

	void Window::update()
	{
		if (m_active) glfwSwapBuffers(m_glfwWindow);
	}

	bool Window::stillRunning()
	{
		return !glfwWindowShouldClose(m_glfwWindow);
	}
}