#include "stdafx.h"
#include "Object.h"
#include "Camera.h"
#include "TextureManager.h"
#include "MeshManager.h"

namespace OGL
{
	Object::Object(const std::string& name, MeshManager& meshManager, TextureManager& textureManager)
	{
		m_mesh = meshManager.getMesh(name, textureManager);
		m_objectMatrix = glm::mat4(1);
	}

	Object::~Object()
	{
	}

	void Object::render(TextureManager& tm)
	{
		m_mesh->render(tm);
	}

	void Object::renderDepth()
	{
		m_mesh->renderDepth();
	}

	void Object::renderBV()
	{
		m_mesh->renderAABB();
	}

	CullType Object::isInFrustum(const Camera& camera)
	{
		CullType cull = INSIDE;

		auto& center = glm::vec3(m_objectMatrix * glm::vec4(m_mesh->m_AABB.m_center, 1.f));
		auto& radius = glm::vec3(m_objectMatrix * glm::vec4(m_mesh->m_AABB.m_radius, 0.f));

		for (auto& plane : camera.m_frustum.m_planes)
		{
			f32 d = glm::dot(center, plane.m_normal);
			f32 r = glm::dot(radius, glm::vec3(std::fabs(plane.m_normal[0]), std::fabs(plane.m_normal[1]), std::fabs(plane.m_normal[2])));

			f32 d_p_r = d + r;
			f32 d_m_r = d - r;

			if (d_p_r + plane.m_dist < 0)
			{
				cull = OUTSIDE;
				break;
			}
			else if (d_m_r + plane.m_dist < 0)
			{
				cull = INTERSECT;
			}
		}
		return cull;
	}
}