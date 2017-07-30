#include "stdafx.h"
#include "Camera.h"

namespace OGL
{
	Camera::Camera()
	{
		m_cameraPosition = glm::vec3(0, 3, 0);
		m_cameraLookAt = m_cameraPosition + glm::vec3(1, 0, 0);
		m_cameraUp = glm::vec3(0, 1, 0);
		m_fov = 45;
		m_rotationQuat = glm::quat(1, 0, 0, 0);
		m_cameraPitch = 0.f;
		m_cameraYaw = 0.f;
		m_firstMouse = true;
		m_cameraSpeed = 10.f;
	}
	Camera::~Camera()
	{
	}

	void Camera::update(f32 deltaTime, f32 totalTime)
	{
		m_deltaTime = deltaTime;
		m_totalTime = totalTime;
		m_cameraDirection = glm::normalize(m_cameraLookAt - m_cameraPosition);
		m_projection = glm::perspective(glm::radians(m_fov), m_aspect, m_nearPlane, m_farPlane);
		// determine axis for pitch rotation
		glm::vec3 axis = glm::cross(m_cameraDirection, m_cameraUp);
		// Compute quaternion for pitch based on the camera pitch angle
		glm::quat pitch_quat = glm::angleAxis(m_cameraPitch, axis);
		// Determine heading quaternion from the camera up vector and the heading angle
		glm::quat heading_quat = glm::angleAxis(m_cameraYaw, m_cameraUp);
		// Add the two quaternions
		glm::quat temp = glm::cross(pitch_quat, heading_quat);
		temp = glm::normalize(temp);
		// update the direction from the quaternion
		m_cameraDirection = glm::rotate(temp, m_cameraDirection);
		// Set the look at
		m_cameraLookAt = m_cameraPosition + m_cameraDirection;
		// Damping for smooth camera
		m_cameraYaw *= .5;
		m_cameraPitch *= .5;
		//compute the MVP
		m_view = glm::lookAt(m_cameraPosition, m_cameraLookAt, m_cameraUp);
		m_model = glm::mat4(1) * glm::translate(glm::vec3(m_cameraPosition));
		m_MVP = m_projection * m_view;
		m_frustum.update(m_MVP);
	}

	void Camera::setPosition(glm::vec3 pos)
	{
		m_cameraPosition = pos;
	}

	void Camera::setLookAt(glm::vec3 pos)
	{
		m_cameraLookAt = pos;
	}

	void Camera::setFOV(f32 fov)
	{
		m_fov = fov;
	}
	void Camera::setViewport(s32 width, s32 height)
	{
		m_windowWidth = width;
		m_windowHeight = height;
		m_aspect = f32(width) / f32(height);
	}
	void Camera::setClipping(f32 nearP, f32 farP)
	{
		m_nearPlane = nearP;
		m_farPlane = farP;
	}

	void Camera::setCameraSpeed(f32 speed)
	{
		m_cameraSpeed = speed;
	}

	void Camera::getMatrices(glm::mat4 &P, glm::mat4 &V, glm::mat4 &M)
	{
		P = m_projection;
		V = m_view;
		M = m_model;
	}

	void Camera::handleKeypress(u32 key, u32 action)
	{
		glm::vec3 right = glm::cross(m_cameraDirection, m_cameraUp);

		// Move forward
		if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
			m_cameraPosition += m_cameraDirection * m_deltaTime * m_cameraSpeed;
		}
		// Move backward
		if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
			m_cameraPosition -= m_cameraDirection * m_deltaTime * m_cameraSpeed;
		}
		// Move up
		if (key == GLFW_KEY_P && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
			m_cameraPosition += m_cameraUp * m_deltaTime * m_cameraSpeed;
		}
		// Move down
		if (key == GLFW_KEY_O && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
			m_cameraPosition -= m_cameraUp * m_deltaTime * m_cameraSpeed;
		}
		// Strafe right
		if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
			m_cameraPosition += right * m_deltaTime * m_cameraSpeed;
		}
		// Strafe left
		if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
			m_cameraPosition -= right * m_deltaTime * m_cameraSpeed;
		}
		m_cameraLookAt = m_cameraPosition + m_cameraDirection;
	}

	void Camera::handleCameraRotation(f32 xpos, f32 ypos)
	{
		if (m_firstMouse)
		{
			m_lastX = xpos;
			m_lastY = ypos;
			m_firstMouse = false;
		}

		f32 xoffset = xpos - m_lastX;
		f32 yoffset = m_lastY - ypos;
		m_lastX = xpos;
		m_lastY = ypos;
		f32 sensitivity = 0.01f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		m_cameraYaw -= xoffset;
		m_cameraPitch += yoffset;

		if (m_cameraPitch > 89.0f)
			m_cameraPitch = 89.0f;
		if (m_cameraPitch < -89.0f)
			m_cameraPitch = -89.0f;

		glm::vec3 front;
		front.x = std::sin(glm::radians(m_cameraYaw)) * std::cos(glm::radians(m_cameraPitch));
		front.y = std::sin(glm::radians(m_cameraPitch));
		front.z = std::cos(glm::radians(m_cameraPitch)) * std::cos(glm::radians(m_cameraYaw));
		m_cameraDirection = glm::normalize(front);
	}

	void Frustum::update(const glm::mat4& vp)
	{
		m_planes[LEFT_P] = glm::row(vp, 3) + glm::row(vp, 0);
		m_planes[RIGHT_P] = glm::row(vp, 3) - glm::row(vp, 0);
		m_planes[DOWN_P] = glm::row(vp, 3) + glm::row(vp, 1);
		m_planes[TOP_P] = glm::row(vp, 3) - glm::row(vp, 1);
		m_planes[NEAR_P] = glm::row(vp, 3) + glm::row(vp, 2);
		m_planes[FAR_P] = glm::row(vp, 3) - glm::row(vp, 2);

		for (auto& p : m_planes)
		{
			f32 l = glm::length(p.m_normal);
			p.m_plane = p.m_plane / l;
		}
	}

	Frustum::Frustum() 
	{
	}

	glm::vec2 Camera::jitterProjectionMatrix(s32 sampleCount, f32 jitterAASigma, f32 width, f32 height)
	{
		// Per-frame jitter to camera for AA
		const s32 frameNum = sampleCount + 1; // Add 1 since otherwise first sample is an outlier

		f32 u1 = halton(frameNum, 2.0f);
		f32 u2 = halton(frameNum, 3.0f);
		// Gaussian sample
		f32 phi = 2.0f*glm::pi<f32>()*u2;

		f32 r = jitterAASigma*sqrtf(-2.0f*log(std::max(u1, 1e-7f)));
		f32 x = r*cos(phi);
		f32 y = r*sin(phi);

		m_projection[2][0] += x*2.0f / width;
		m_projection[2][1] += y*2.0f / height;

		return glm::vec2(x, y);
	}
}