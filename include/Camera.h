#pragma once
#include "Util.h"

namespace OGL
{
	class Plane
	{
	public:
		union
		{
			glm::vec4 m_plane;
			struct
			{
				glm::vec3 m_normal;
				f32 m_dist;
			};
		};

		Plane() {};
		Plane(glm::vec3 n, f32 d) : m_normal{ n }, m_dist{ d } {};
		~Plane() {};
		f32& operator [] (u32 i) { return m_plane[i]; };
		const f32& operator[] (u32 i) const { return m_plane[i]; };
		void operator = (glm::vec4 p) { m_plane = p; };
		void normalize() { m_normal = glm::normalize(m_normal); }
	};

	class Frustum
	{
	public:
		enum Planes
		{
			NEAR_P,
			FAR_P,
			LEFT_P,
			RIGHT_P,
			TOP_P,
			DOWN_P,
			NB_PLANES
		};

		std::array<Plane, NB_PLANES> m_planes;
		//std::array<u32, NB_BUFFER> m_vbo;
		//u32 m_vao;

		Frustum();
		void update(const glm::mat4& combo);
	};

	enum class CameraDirection : u8
	{
		UP, DOWN, LEFT, RIGHT, FORWARD, BACK
	};

	class Camera {
	public:
		Camera();
		~Camera();
		void update(f32 deltaTime, f32 totalTime);
		void setPosition(glm::vec3 pos);
		void setLookAt(glm::vec3 pos);
		void setFOV(f32 fov);
		void setViewport(s32 width, s32 height);
		void setClipping(f32 nearP, f32 farP);
		void setCameraSpeed(f32 speed);
		void getMatrices(glm::mat4 &P, glm::mat4 &V, glm::mat4 &M);
		glm::mat4 getMVP() { return m_MVP; };
		glm::mat4 getView() { return m_view; };
		glm::mat4 getProjection() { return m_projection; };
		void handleKeypress(u32 key, u32 action);
		void handleCameraRotation(f32 xpos, f32 ypos);
		glm::vec2 jitterProjectionMatrix(s32 sampleCount, f32 jitterAASigma, f32 width, f32 height);

		Frustum m_frustum;
		glm::mat4 m_projection;
		glm::mat4 m_view;
		glm::mat4 m_model;
		glm::mat4 m_MVP;
		glm::quat m_rotationQuat;
		glm::vec3 m_cameraPosition;
		glm::vec3 m_cameraLookAt;
		glm::vec3 m_cameraUp;
		glm::vec3 m_cameraDirection;
		s32 m_windowWidth;
		s32 m_windowHeight;
		f32 m_aspect;
		f32 m_fov;
		f32 m_nearPlane;
		f32 m_farPlane;
		f32 m_cameraPitch;
		f32 m_cameraYaw;
		f32 m_deltaTime;
		f32 m_totalTime;
		f32 m_lastX, m_lastY;
		f32 m_cameraSpeed;
		bool m_firstMouse;
	};
}