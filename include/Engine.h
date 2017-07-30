#pragma once
#include "Renderer.h"
#include "Window.h"

namespace OGL
{
	class Engine : Uncopiable
	{
	public:
		Engine();
		~Engine();
		void printPlatformInfo();
		void initGL();
		void run();
		void renderScene();
		void handleKeypress(s32 key, s32 scancode, s32 action, s32 mods);
		void handleMouseButton(s32 button, s32 action, s32 mods);
		void handleCameraMovement(f32 xpos, f32 ypos);
		void resizeWindow(s32 w, s32 h);
		void triggerScreenshot();
		void picking(f32 x, f32 y);

		Renderer m_renderer;
		Window m_window;
	};
}