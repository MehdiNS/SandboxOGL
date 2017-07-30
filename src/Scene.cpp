#include "stdafx.h"
#include "Scene.h"
#include "Util.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "MeshManager.h"
#include "ShaderManager.h"
#include "Camera.h"

namespace OGL
{
	Scene::Scene(LightManager& lm, TextureManager& tm, MeshManager& mm, ShaderManager& sm)
	{
		createAssets(lm, tm, mm, sm);
		generateUniformBuffers(lm, sm);
	}

	Scene::~Scene()
	{

	}

	void Scene::createAssets(LightManager& lm, TextureManager& tm, MeshManager& mm, ShaderManager& sm)
	{
		// Init framebuffer
		m_frameBuffer = std::make_unique<FrameBuffer>(tm, 640, 480, false);
		m_shadowFrameBuffer = std::make_unique<SMFrameBuffer>(tm, 512);
		//m_snowFrameBuffer = std::make_unique<SnowFrameBuffer>(tm, 512, 512);

		// TODO : retrive shaders automatically in their folder
		// Load shaders
		sm.getShader("Bilateral");
		sm.getShader("Ssao");
		sm.getShader("Quad");
		sm.getShader("GeometryPass");
		//sm.getShader("GeometryPassTerrain");
		sm.getShader("Cube");
		sm.getShader("LightingPassDL");
		sm.getShader("Skybox");
		sm.getShader("Blit");
		sm.getShader("Combine");
		sm.getShader("Gamma");
		sm.getShader("Fxaa");
		sm.getShader("Snow");
		sm.getShader("Blur");
		sm.getShader("Terrain");
		sm.getShader("Grass");
		sm.getShader("EquirectangleToCubemap");
		sm.getShader("Line");
		sm.getShader("Shadow");
		sm.getShader("GeometryPassOcean");
		sm.getShader("Resolve");
		//sm.getShader("Ssr");
		sm.getShader("LightingPassAL");
		sm.getShader("AreaLight");
		sm.getShader("Irradiance");

		m_sceneOptions.m_hour = 12.f;
		m_sceneOptions.m_wetness = 0.f;
		m_sceneOptions.m_exposure = 1.f;
		
		// Load lights
		lm.AddLight(Light(LightDataLayout(glm::vec3(-1.f, 1.f, 1.f), 1.0f, 1.0f, glm::vec3(0.f, 3.f, 1.f), glm::vec3(1.f, 1.f, 1.f))), tm);
	
		// Meshes
		//Object object0("asset/Plane/plane.obj", mm, tm);
		//object0.m_objectMatrix = glm::translate(glm::vec3(0, 0.1, 0)) * glm::scale(glm::vec3(10.f));
		//m_objectList.emplace_back(object0);
		Object object3("asset/Sponza/sponza.obj", mm, tm);
		object3.m_objectMatrix = glm::translate(glm::vec3(0, 0, 0)) * glm::scale(glm::vec3(0.01f)); //Sponza is authored in cm
		m_objectList.emplace_back(object3);

		//u32 ltcAmpId = tm.createTexture(ASSET_TEXTURE2D_MIP_OFF_FLOAT, "ltc_amp.dds");
		//u32 ltcMatId = tm.createTexture(ASSET_TEXTURE2D_MIP_OFF_FLOAT, "ltc_mat.dds");
		//m_areaLightList.emplace_back(AreaLight{});
		//m_areaLightList[0].m_scale = 10.f;
		//glm::mat4 scale = glm::scale(glm::vec3(m_areaLightList[0].m_scale));
		//glm::mat4 rotation = glm::rotate(glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
		//glm::mat4 translation = glm::translate(glm::vec3(77, 10, -3));
		//m_areaLightList[0].m_transform = translation*rotation*scale;
		//m_areaLightList[0].m_ltcAmpId = ltcAmpId;
		//m_areaLightList[0].m_ltcMatId = ltcMatId;
		
		//Object object2("asset/Plane/plane.obj", mm, tm);
		//object2.m_mesh->m_albedoMapsId[0] = m_snowFrameBuffer->m_heightmapId[0];
		//object2.m_objectMatrix = glm::translate(glm::vec3(0, 0, 0)) * glm::scale(glm::vec3(10.f));
		//m_objectList.emplace_back(object2);
		//u32 heightMapId = tm.createTexture(ASSET_TEXTURE2D_MIP_ON, "asset/Plane/Heightmap.jpg");

		Object object("asset/Sphere/sphere.obj", mm, tm);
		object.m_objectMatrix = glm::translate(glm::vec3(0, 2, 0)) * glm::scale(glm::vec3(0.5f));
		m_objectList.emplace_back(object);

		//Object object3("asset/POM_Plane/plane.obj", mm, tm);
		//object3.m_mesh->m_heightMapsId[1] = m_snowFrameBuffer->m_heightmapId1;
		////object3.m_mesh->m_heightMapsId[1] = heightMapId;
		//object3.m_objectMatrix = glm::translate(glm::vec3(0, 0, 0)) * glm::scale(glm::vec3(10.f));
		//m_objectList.emplace_back(object3);

		// Load misc
		m_terrain = std::make_unique<Terrain>(tm);
		m_grass = std::make_unique<Grass>(tm);
		m_ocean = std::make_unique<Ocean>(tm);
	}

	void Scene::prepareFrame()
	{
		// Wipe the drawing surface clear
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer->m_fbo);
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClearStencil(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glViewport(0, 0, 640, 480);
	}

	void Scene::updateUBOStatic(LightManager& lm, Camera& camera)
	{
		m_matrixData.projection = camera.getProjection();
		m_matrixData.view = camera.getView();
		m_matrixData.viewProjInv = glm::inverse(camera.getProjection() * camera.getView());
		m_matrixData.normal = glm::transpose(glm::inverse(m_matrixData.view));
		m_matrixData.cameraPos = glm::vec4(camera.m_cameraPosition, camera.m_nearPlane);

		auto hour = m_sceneOptions.m_hour;
		auto sunPos = computeSunPosition(hour, 140, 48.858844f, 2.294602, 2);
		auto sunTheta = sunPos.first;
		auto sunPhi = sunPos.second;
		glm::vec3 sunDirection = glm::normalize(spherical(sunTheta, sunPhi));
		f32 near_plane = 0.1f, far_plane = 40.f;
		lm.m_lightsInScene[0].setLightPos(sunDirection);
		glm::vec3 lightPos = glm::normalize(sunDirection);
		lightPos = lightPos * glm::vec3(30.f);
		glm::mat4 lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		lm.m_lightsInScene[0].m_LightSpaceMatrix = lightProjection * lightView;
		m_matrixData.sunDir = glm::vec4(sunDirection, camera.m_farPlane);
		m_matrixData.skyData = glm::vec4(m_sceneOptions.m_wetness, camera.m_totalTime, camera.m_deltaTime, 0);

		GLvoid* p;
		glBindBuffer(GL_UNIFORM_BUFFER, m_uboStatic);
		p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
		memcpy(p, &m_matrixData, sizeof(m_matrixData));
		glUnmapBuffer(GL_UNIFORM_BUFFER);

		//glBindBuffer(GL_UNIFORM_BUFFER, m_uboLightArray);
		//p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
		//memcpy(p, &lm.m_lightsInScene, sizeof(lm.m_lightsInScene));
		//glUnmapBuffer(GL_UNIFORM_BUFFER);
	}

	void Scene::generateUniformBuffers(LightManager& lm, ShaderManager& sm)
	{
		glGenBuffers(1, &m_uboStatic);
		glBindBuffer(GL_UNIFORM_BUFFER, m_uboStatic);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(m_matrixData), &m_matrixData, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		//glGenBuffers(1, &m_uboLightArray);
		//glBindBuffer(GL_UNIFORM_BUFFER, m_uboLightArray);
		//glBufferData(GL_UNIFORM_BUFFER, sizeof(lm.m_lightsInScene), &lm.m_lightsInScene, GL_DYNAMIC_DRAW);
		//glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glGenBuffers(1, &m_uboLight);
		glBindBuffer(GL_UNIFORM_BUFFER, m_uboLight);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(m_pointLightData), &m_pointLightData, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glGenBuffers(1, &m_uboSky);
		glBindBuffer(GL_UNIFORM_BUFFER, m_uboSky);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(m_sceneOptions.m_hour), &m_sceneOptions.m_hour, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		for (auto& shader : sm.m_shaders)
		{
			u32 program;
			u32 binding_point_index;

			if (shader.second->isValidUBO("matrix_data"))
			{
				program = shader.second->m_program;
				binding_point_index = 2;
				glUniformBlockBinding(program, glGetUniformBlockIndex(program, "matrix_data"), binding_point_index);
				glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_index, m_uboStatic);
			}
			//if (shader.second->isValidUBO("light_data"))
			//{
			//	binding_point_index = 4;
			//	glUniformBlockBinding(program, glGetUniformBlockIndex(program, "light_data"), binding_point_index);
			//	glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_index, m_uboLightArray);
			//}
			if (shader.second->isValidUBO("point_light_data"))
			{
				binding_point_index = 5;
				glUniformBlockBinding(program, glGetUniformBlockIndex(program, "point_light_data"), binding_point_index);
				glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_index, m_uboLight);
			}
			if (shader.second->isValidUBO("sun_data"))
			{
				binding_point_index = 1;
				glUniformBlockBinding(program, glGetUniformBlockIndex(program, "sun_data"), binding_point_index);
				glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_index, m_uboSky);
			}
		}
	}
}