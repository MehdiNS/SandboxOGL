#include "stdafx.h"
#include "Engine.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb\stb_image_write.h>
#include "Util.h"

namespace OGL
{
	Engine::Engine() :
		m_window{},
		m_renderer{}
	{
	}

	Engine::~Engine()
	{
	}

	void Engine::printPlatformInfo()
	{
		/* get version info */
		const u8* renderer;
		const u8* version;
		renderer = glGetString(GL_RENDERER); /* get renderer string */
		version = glGetString(GL_VERSION); /* version as a string */
		DEBUG_ONLY(std::cout << "Renderer:" << renderer << "\n");
		DEBUG_ONLY(std::cout << "OpenGL version supported " << version << std::endl);
	}

	void Engine::initGL()
	{
		// start GLAD extension handler
		if (!gladLoadGL())
			exit(-1);

		printPlatformInfo();

		// Debug Output
		s32 flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback((GLDEBUGPROC)glDebugOutput, nullptr);
			glDebugMessageControl(GL_DEBUG_SOURCE_API,
				GL_DEBUG_TYPE_ERROR,
				GL_DEBUG_SEVERITY_HIGH,
				0, nullptr, GL_TRUE);
		}

		glEnable(GL_CULL_FACE); // cull face
		glCullFace(GL_BACK); // cull back face
		glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

							 /* tell GL to only draw onto a pixel if the shape is closer to the viewer */
		glEnable(GL_DEPTH_TEST); /* enable depth-testing */
								 /* with LESS depth-testing interprets a smaller value as "closer" */
		glDepthFunc(GL_LESS);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	void Engine::run()
	{
		initGL();
		ImGui_ImplGlfwGL3_Init(m_window.m_glfwWindow, true);
		m_renderer.initScene();
		renderScene();
		ImGui_ImplGlfwGL3_Shutdown();
	}

	void Engine::renderScene()
	{
		f64 t = 0.0;
		f64 dt = 1.0 / 60.0;
		f64 currentTime = glfwGetTime();

		while (m_window.stillRunning())
		{
			f64 newTime = glfwGetTime();
			f64 frameTime = newTime - currentTime;
			currentTime = newTime;

			while (frameTime > 0.0)
			{
				f32 deltaTime = std::min(frameTime, dt);
				frameTime -= deltaTime;
				t += deltaTime;
			}

			m_renderer.prepareScene(dt, t);
			m_renderer.renderScene();
			glfwPollEvents();
			m_window.update();
		}
	}

	void Engine::handleKeypress(s32 key, s32 scancode, s32 action, s32 mods)
	{
		// User hit ESC? Set the window to close
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(m_window.m_glfwWindow, GL_TRUE);
		}
		else if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else if (key == GLFW_KEY_F4 && action == GLFW_PRESS)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		}
		else if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
		{
			m_renderer.m_shaderManager.updateAll();
		}
		else if (key == GLFW_KEY_F6 && action == GLFW_PRESS)
		{
			// TODO : urghh, doing two cast for that is ugly
			m_renderer.m_debugOptions.m_debugMode = static_cast<DebugMode>((static_cast<int>(m_renderer.m_debugOptions.m_debugMode) + 1) % DebugMode::NB_DEBUGMODE);
		}
		else if (key == GLFW_KEY_PRINT_SCREEN && action == GLFW_PRESS)
		{
			triggerScreenshot();
		}
		else
		{
			// Camera movement
			m_renderer.m_camera.handleKeypress(key, action);
		}
	}

	void Engine::handleMouseButton(s32 button, s32 action, s32 mods)
	{
		f64 xpos, ypos;
		glfwGetCursorPos(m_window.m_glfwWindow, &xpos, &ypos);

		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{
			if (xpos < m_window.m_windowWidth / 2.5)
			{
				m_renderer.m_debugOptions.m_leftClicking = false;
				m_renderer.m_debugOptions.m_rightClicking = false;
				return;
			}

			m_renderer.m_debugOptions.m_leftClicking = true;
			m_renderer.m_debugOptions.m_rightClicking = false;
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		{
			m_renderer.m_debugOptions.m_leftClicking = false;
			m_renderer.m_debugOptions.m_rightClicking = true;
		}
		else
		{
			m_renderer.m_debugOptions.m_leftClicking = false;
			m_renderer.m_debugOptions.m_rightClicking = false;
		}

		picking(static_cast<f32>(xpos), static_cast<f32>(ypos));
	}

	void Engine::handleCameraMovement(f32 xpos, f32 ypos)
	{
		if (m_renderer.m_debugOptions.m_leftClicking)
			m_renderer.m_camera.handleCameraRotation(xpos, ypos);
	}

	void Engine::resizeWindow(s32 w, s32 h)
	{
		if (w != 0 && h != 0)
		{
			m_window.m_active = true;
			m_window.m_windowWidth = w;
			m_window.m_windowHeight = h;
			m_renderer.m_camera.setViewport(w, h);
			m_renderer.m_camera.update(0.f, 0.f);
		}
		else
		{
			m_window.m_active = false;
		}
	}

	void Engine::triggerScreenshot()
	{
		std::vector<u8> buffer(m_window.m_windowWidth * m_window.m_windowHeight * 4);
		glReadPixels(0, 0, m_window.m_windowWidth, m_window.m_windowHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
		s8 name[1024];
		auto t = time(NULL);
		sprintf_s(name, "screenshot_%ld.png", static_cast<long>(t));
		for (u32 i = 3; i < buffer.size(); i = i + 4)
			buffer[i] = 255;
		u8* last_row = buffer.data() + (m_window.m_windowWidth * 4 * (m_window.m_windowHeight - 1));
		if (!stbi_write_png(name, m_window.m_windowWidth, m_window.m_windowHeight, 4, last_row, -4 * m_window.m_windowWidth)) {
			fprintf(stderr, "ERROR: could not write screenshot file %s\n", name);
		}
	}

	void Engine::picking(f32 x, f32 y)
	{
		u8 color[4] = { 0 };
		f32 depth, depthN, depthE;
		u32 index;

		glReadPixels((u32)x, m_window.m_windowHeight - (u32)y - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color);

		glReadPixels((u32)x, m_window.m_windowHeight - (u32)y - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
		glReadPixels((u32)x + 1, m_window.m_windowHeight - (u32)y - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthE);
		glReadPixels((u32)x, m_window.m_windowHeight - ((u32)y + 1) - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthN);

		glReadPixels((u32)x, m_window.m_windowHeight - (u32)y - 1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);

		f32 linearDepth = linearDepthWS(depth, m_renderer.m_camera.m_nearPlane, m_renderer.m_camera.m_farPlane);
		f32 linearDepthNorth = linearDepthWS(depthN, m_renderer.m_camera.m_nearPlane, m_renderer.m_camera.m_farPlane);
		f32 linearDepthEst = linearDepthWS(depthE, m_renderer.m_camera.m_nearPlane, m_renderer.m_camera.m_farPlane);

		DEBUG_ONLY(std::cout << "Pixel " << x << " : " << y << "\n"
			<< "Color " << (u32)color[0] << " : " << (u32)color[1] << " : " << (u32)color[2] << " : " << "\n"
			<< "Depth " << depth << "\n"
			<< "Linearize Depth " << linearDepth << "\n"
			<< "Stencil " << index << "\n" << std::endl);

		m_renderer.m_debugOptions.m_pickedObj = index;

		if (m_renderer.m_debugOptions.m_rightClicking)
		{
			glm::vec3 cameraVec = m_renderer.m_camera.m_cameraDirection * linearDepth;
			glm::vec3 posWS = m_renderer.m_camera.m_cameraPosition + cameraVec;

			glm::vec4 viewport = glm::vec4(0, 0, 640, 480);
			glm::vec3 wincoord = glm::vec3(x, 480 - y - 1, depth);
			glm::vec3 objcoord = glm::unProject(wincoord, m_renderer.m_camera.getView(), m_renderer.m_camera.getProjection(), viewport);
			glm::vec3 wincoordE = glm::vec3(x + 1, 480 - y - 1, depthE);
			glm::vec3 objcoordE = glm::unProject(wincoordE, m_renderer.m_camera.getView(), m_renderer.m_camera.getProjection(), viewport);
			glm::vec3 wincoordN = glm::vec3(x, 480 - (y + 1) - 1, depthN);
			glm::vec3 objcoordN = glm::unProject(wincoordN, m_renderer.m_camera.getView(), m_renderer.m_camera.getProjection(), viewport);

			glm::vec3 contactToEye = glm::normalize(m_renderer.m_camera.m_cameraPosition - objcoord);
			glm::vec3 N = glm::normalize(glm::cross(objcoordN - objcoord, objcoordE - objcoord));
			if (glm::dot(N, contactToEye) < 0)
				N *= -1.;

			glm::vec3 V1 = glm::normalize(orthogonal(N));
			glm::vec3 V2 = glm::normalize(glm::cross(V1, N));
			glm::mat4 rotation{ glm::vec4(V1, 0), glm::vec4(N, 0), glm::vec4(V2, 0), glm::vec4(0,0,0,1) };

			Object object4("asset/Cube/cube.obj", m_renderer.m_meshManager, m_renderer.m_textureManager);
			m_renderer.m_scene->m_decalList.emplace_back(object4);

			glm::mat4 translation = glm::translate(objcoord);
			glm::mat4 scale = glm::scale(glm::vec3(1.f));

			m_renderer.m_scene->m_decalList.back().m_objectMatrix = translation * rotation * scale;
		}
	}
}