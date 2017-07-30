#pragma once
#include "Mesh.h"

namespace OGL
{
	enum CullType
	{
		OUTSIDE = 0,
		INTERSECT,
		INSIDE
	};

	class Camera;
	class TextureManager;
	class MeshManager;

	class Object
	{
	public:
		Object(const std::string& name, MeshManager& meshManager, TextureManager& textureManager);
		~Object();
		void render(TextureManager& tm);
		void renderDepth();
		void renderBV();
		CullType isInFrustum(const Camera& camera);
		
		glm::mat4 m_objectMatrix;
		Mesh* m_mesh;
	};
}