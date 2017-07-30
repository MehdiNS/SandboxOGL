#pragma once 
#include "Engine.h"

namespace OGL
{
	Engine engine;

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		engine.handleKeypress(key, scancode, action, mods);
	}

	static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
	{
		engine.resizeWindow(width, height);
	}

	static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
	{
		engine.handleMouseButton(button, action, mods);
	}

	static void mouse_movement_callback(GLFWwindow* window, f64 xpos, f64 ypos)
	{
		engine.handleCameraMovement(static_cast<f32>(xpos), static_cast<f32>(ypos));
	}
}

int main() {
	//Set callbacks
	using namespace OGL;
	glfwSetKeyCallback(engine.m_window.m_glfwWindow, key_callback);
	glfwSetWindowSizeCallback(engine.m_window.m_glfwWindow, framebuffer_size_callback);

	glfwSetMouseButtonCallback(engine.m_window.m_glfwWindow, mouseButtonCallback);
	glfwSetInputMode(engine.m_window.m_glfwWindow, GLFW_STICKY_MOUSE_BUTTONS, 1);

	glfwSetInputMode(engine.m_window.m_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(engine.m_window.m_glfwWindow, mouse_movement_callback);

	engine.run();
	return 0;
}
