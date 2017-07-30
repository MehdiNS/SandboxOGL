#include "stdafx.h"
#include "Renderer.h"
#include "Util.h"
#include "Axis.h"
#include "Timer.h"
#include "Object.h"
#include "FrameBuffer.h"
#include "ShadowFrameBuffer.h"
#include "SnowFrameBuffer.h"

namespace OGL
{
	bool Renderer::g_firstFrame = true;

	Renderer::Renderer() :
		m_debugOptions{ 0, DebugMode::FinalFrame },
		m_renderingOptions{ true, true, true, false, false, false, false }, // TODO : Use enums to prevent this mess
		m_areaLightOptions{ {1.f,1.f,1.f}, 1.f },
		m_taaOptions{ 0.001f },
		m_sampleCount{ 0 },
		m_scene{ nullptr },
		m_saoOptions{ 1.f, 0.1f, 1.f }
	{
	};

	Renderer::~Renderer()
	{
	};

	void Renderer::initScene()
	{
		// Construct the scene
		m_scene = std::make_unique<Scene>(m_lightManager, m_textureManager, m_meshManager, m_shaderManager);

		// Prepare postFX
		m_postEffectManager.init(m_textureManager, m_textureManager.getTextureHandle(m_scene->m_frameBuffer->m_finalTextureCurrentId));

		// Init camera. Update() creates camera matrices and frustum
		m_camera.setViewport(640, 480);
		m_camera.setClipping(0.1f, 1000.f);
		m_camera.update(0., 0.);

		m_uboLight = m_scene->m_uboLight;
		initIBL();
	}

	void Renderer::prepareScene(f32 dt, f32 t)
	{
		m_camera.update(dt, t);
		auto res = m_camera.jitterProjectionMatrix(
			m_sampleCount / NUM_SAMPLES,
			m_taaOptions.m_jitterAASigma,
			640.f,
			480.f);

		// Hacky way to move the ball for the snow demo
		//m_scene->m_objectList[0].m_objectMatrix[3] =
		//	glm::vec4(
		//		5 * glm::cos(0.15*t) * glm::cos(0.0375874*t),
		//		0.51+glm::abs(0.10*glm::sin(0.05*t)),
		//		5.* glm::sin(0.25*t) * glm::sin(0.03875874*t), 1);

		m_scene->prepareFrame();
		initGui();
	}

	void Renderer::renderScene()
	{
		Timer tFrame{ m_timerValues.m_frame };

		if (m_renderingOptions.m_renderShadow)
		{
			Timer tShadow{ m_timerValues.m_shadow };
			renderShadow();
		}

		//{
		//	Timer timer{ m_timerValues.m_prepareSnow };
		//	prepareSnow();
		//}

		{
			//Update UBO (per-frame data)
			Timer timer{ m_timerValues.m_uboUpdate };
			m_scene->updateUBOStatic(m_lightManager, m_camera);
		}

		{
			//Render Opaque
			Timer tOpaque{ m_timerValues.m_opaque };
			renderOpaque();
		}
		{
			//Render sky
			Timer tSky{ m_timerValues.m_sky };
			renderSkybox();
		}
		{
			//Render AABB of picked object
			//renderPickedBoundingVolume();
			//renderAxis();
		}

		// When we get here, the depth buffer is already populated and the stencil pass
		// depends on it, but it does not write to it.
		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);

		if (m_renderingOptions.m_renderPostFx)
		{
			Timer tPostFx{ m_timerValues.m_postFX };
			renderPostEffects();
		}

		// Render ImGui
		chooseDebugMode();
		if (m_renderingOptions.m_renderImGui)
		{
			renderImGui();
			renderGui();
		}

		// Finalize frame
		finalize();

		m_sampleCount += NUM_SAMPLES;
		g_firstFrame = false;
		auto& fb = m_scene->m_frameBuffer;

		std::swap(fb->m_finalTextureCurrentId, fb->m_finalTextureLastId);
		glBindFramebuffer(GL_FRAMEBUFFER, fb->m_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, m_textureManager.getTextureHandle(fb->m_finalTextureCurrentId), 0);
	}

	void Renderer::renderOpaque()
	{
		auto& fb = m_scene->m_frameBuffer;

		auto& objList = m_scene->m_objectList;
		auto& ocean = m_scene->m_ocean;
		{
			Timer timer{ m_timerValues.m_geometry };
			fb->startFrame(m_textureManager);

			// Only the geometry pass updates the depth buffer
			glDepthMask(GL_TRUE);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glEnable(GL_CULL_FACE); // cull face
			glCullFace(GL_BACK); // cull back face

			geometryPass();

			// When we get here the depth buffer is already populated and the stencil pass
			// depends on it, but it does not write to it.
			glDisable(GL_STENCIL_TEST);
			glDepthMask(GL_FALSE);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			{
				Timer timer{ m_timerValues.m_decal };
				renderSSD();
			}
		}

		if (m_renderingOptions.m_renderSSAO)
		{
			Timer timer{ m_timerValues.m_ssao };
			ambientPass();
		}

		{
			Timer timer{ m_timerValues.m_light };
			if (objList.size() != 0 || ocean != nullptr)
				shadingPass();
		}

		{
			Timer timer{ m_timerValues.m_resolve };
			resolvePass();
		}

		{
			//Timer timer{ m_timerValues.m_ssr };
			//SSRPass();
		}

		{
			Timer timer{ m_timerValues.m_physicalAL };
			renderAreaLight();
		}
	}

	void Renderer::geometryPass()
	{
		auto& fb = m_scene->m_frameBuffer;
		auto& objList = m_scene->m_objectList;
		auto& ocean = m_scene->m_ocean;
		auto& terrain = m_scene->m_terrain;

		fb->bindForGeometryPass();
		{
			auto& shader = m_shaderManager.getShader("GeometryPass");
			shader.bind();

			u32 nbObj = objList.size();
			for (u32 i = 0; i < nbObj; ++i)
			{
				if (objList[i].isInFrustum(m_camera) != OUTSIDE) // TODO : Acton warning
				{
					s32 useReliefMap = objList[i].m_mesh->m_heightMapsId[1] > 0 ? 1 : 0;
					GL(glStencilFunc(GL_ALWAYS, i + 1, -1));
					GL(glUniform1i(shader.getUniformLocation("useParallaxMapping"), useReliefMap));
					GL(glUniformMatrix4fv(shader.getUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(objList[i].m_objectMatrix)));
					objList[i].render(m_textureManager);
				}
			}
			clearBindings();
		}

		if (m_renderingOptions.m_renderOcean)
		{
			auto& shader = m_shaderManager.getShader("GeometryPassOcean");
			shader.bind();
			//glm::mat4 identity = glm::mat4(1);
			//glUniformMatrix4fv(shader.getUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(identity));
			ocean->render(m_textureManager);
		}

		if (m_renderingOptions.m_renderTerrain)
		{
			auto& shader = m_shaderManager.getShader("Terrain");
			shader.bind();
			//auto& identity = glm::mat4(1);
			//GL(glUniformMatrix4fv(shader.getUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(identity)));
			terrain->render(m_textureManager);
		}

		if (m_renderingOptions.m_renderGrass)
		{
			glDisable(GL_CULL_FACE);
			auto& shader = m_shaderManager.getShader("Grass");
			shader.bind();
			m_scene->m_grass->render(m_textureManager);
			glEnable(GL_CULL_FACE);
		}
	}

	void Renderer::ambientPass()
	{
		auto& fb = m_scene->m_frameBuffer;

		// SAO pass
		fb->bindForSSAOPass(m_textureManager);
		auto& saoShader = m_shaderManager.getShader("Ssao");
		saoShader.bind();
		glm::vec3 saoData = glm::vec3(m_saoOptions.m_radius, m_saoOptions.m_bias, m_saoOptions.m_intensity);
		GL(glUniform3fv(saoShader.getUniformLocation("saoData"), 1, glm::value_ptr(saoData)));
		drawQuad();

		// Blur pass
		auto& shader = m_shaderManager.getShader("Bilateral");
		shader.bind();
		for (s32 pass = 0; pass < 2; pass++)
		{
			s32 axis[2] = { pass, 1 - pass };
			GL(glUniform2iv(shader.getUniformLocation("axis"), 1, axis));
			fb->bindForSSAOBlurPass(m_textureManager, pass);
			drawQuad();
		}
	}

	void Renderer::renderSSD()
	{
		auto& fb = m_scene->m_frameBuffer;
		auto& objList = m_scene->m_objectList;
		auto& decalList = m_scene->m_decalList;

		fb->bindForDecalPass(m_textureManager);

		auto& shader = m_shaderManager.getShader("Cube");
		shader.bind();

		u32 nbDecal = decalList.size();
		for (u32 i = 0; i < nbDecal; ++i)
		{
			glUniformMatrix4fv(shader.getUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(decalList[i].m_objectMatrix));
			decalList[i].render(m_textureManager);
		}
	}

	void Renderer::shadingPass()
	{
		auto& fb = m_scene->m_frameBuffer;
		auto& objList = m_scene->m_objectList;
		auto& ocean = m_scene->m_ocean;
		auto& irrMapId = m_scene->m_IBLData.m_irrCubeMapId;
		auto& prefilterMapId = m_scene->m_IBLData.m_prefilterMapId;
		auto& brdfLUTId = m_scene->m_IBLData.m_brdfLUTId;
		auto& areaLightList = m_scene->m_areaLightList;

		GL(glDepthMask(GL_FALSE));
		GL(glDisable(GL_DEPTH_TEST));
		GL(glEnable(GL_BLEND));
		GL(glBlendEquation(GL_FUNC_ADD));
		GL(glBlendFunc(GL_ONE, GL_ONE));

		fb->bindForShadingPass(m_textureManager, irrMapId, prefilterMapId, brdfLUTId);

		for (u32 i = 0; i < m_lightManager.m_nbLights; i++)
		{
			updateUBOLight(m_lightManager.m_lightsInScene[i]);
			m_textureManager.bindTexture(m_lightManager.m_shadowMap[i], GL_TEXTURE0 + FrameBuffer::GBUFFER_SIZE + 1);

			if (m_lightManager.m_lightsInScene[i].isDirectionnal())
			{
				auto& shader = m_shaderManager.getShader("LightingPassDL");
				shader.bind();
				//glUniformMatrix4fv(shader.getUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f));
				drawQuad();
			}
			else
			{
				auto sphere = m_meshManager.getMesh("asset/Sphere/sphere.obj", m_textureManager);
				GL(glEnable(GL_CULL_FACE));
				GL(glCullFace(GL_FRONT));

				auto& shader = m_shaderManager.getShader("LightingPassPL");
				shader.bind();

				glm::mat4 translationMatrix = glm::translate(m_lightManager.m_lightsInScene[i].getLightPos());
				glm::mat4 scalingMatrix = glm::scale(glm::vec3(15.f));
				glm::mat4 modelMatrix = translationMatrix * scalingMatrix;
				GL(glUniformMatrix4fv(shader.getUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix)));
				sphere->render(m_textureManager);

				GL(glCullFace(GL_BACK));
				GL(glDisable(GL_CULL_FACE));
			}
		}

		for (u32 i = 0; i < areaLightList.size(); i++)
		{
			fb->bindForAreaLightingPass(m_textureManager, areaLightList[0].m_ltcAmpId, areaLightList[0].m_ltcMatId);
			auto& shader = m_shaderManager.getShader("LightingPassAL");
			shader.bind();
			std::vector<glm::vec4> points =
			{
				glm::vec4(-1, -1, 0, 1),
				glm::vec4(1, -1, 0, 1),
				glm::vec4(1,  1, 0, 1),
				glm::vec4(-1,  1, 0, 1),
			};

			std::transform(points.begin(), points.end(), points.begin(),
				[&](glm::vec4 p) { return areaLightList[i].m_transform * p; });

			GL(glProgramUniform4fv(shader.m_program, glGetUniformLocation(shader.m_program, "pointsList"), 4, glm::value_ptr(points[0])));
			GL(glUniform1fv(shader.getUniformLocation("intensity"), 1, &(m_areaLightOptions.m_intensity)));
			GL(glUniform3fv(shader.getUniformLocation("color"), 1, &(m_areaLightOptions.m_color[0])));
			m_textureManager.bindTexture(areaLightList[i].m_lightTextureId, GL_TEXTURE0 + FrameBuffer::GBUFFER_SIZE + 3);
			drawQuad();
		}

		GL(glDisable(GL_BLEND));
	}

	void Renderer::updateUBOLight(Light& light)
	{
		GL(glBindBuffer(GL_UNIFORM_BUFFER, m_uboLight));
		GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
		memcpy(p, &light, sizeof(light));
		GL(glUnmapBuffer(GL_UNIFORM_BUFFER));
	}

	void Renderer::resolvePass()
	{
		auto& fb = m_scene->m_frameBuffer;
		fb->bindForFinalPass(m_textureManager);
		auto& shader = m_shaderManager.getShader("Resolve");
		shader.bind();
		drawQuad();
	}

	void Renderer::SSRPass()
	{
		auto& fb = m_scene->m_frameBuffer;
		fb->bindForSSRPass(m_textureManager);
		auto& shader = m_shaderManager.getShader("Ssr");
		shader.bind();
		drawQuad();
	}

	//////////   FORWARD   ////////////

	void Renderer::renderShadow()
	{
		auto& fb = m_scene->m_frameBuffer;
		auto& objList = m_scene->m_objectList;
		auto& shadowfb = m_scene->m_shadowFrameBuffer;

		// Prepare shadow
		GL(glBindFramebuffer(GL_FRAMEBUFFER, shadowfb->m_fbo));
		GL(glViewport(0, 0, 512, 512));
		GL(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
		GL(glClear(GL_DEPTH_BUFFER_BIT));
		GL(glCullFace(GL_BACK));

		auto& savedShadowMap = shadowfb->m_shadowMap;

		// Render objects
		auto& shader = m_shaderManager.getShader("Shadow");
		shader.bind();

		for (u32 i = 0; i < m_lightManager.m_nbLights; ++i)
		{
			GL(glUniformMatrix4fv(shader.getUniformLocation("gMVP"), 1, GL_FALSE, &m_lightManager.m_lightsInScene[i].m_LightSpaceMatrix[0][0]));
			shadowfb->setShadowMap(m_textureManager, m_lightManager.m_shadowMap[i]);
			for (auto& obj : objList)
			{
				GL(glUniformMatrix4fv(shader.getUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(obj.m_objectMatrix)));
				obj.renderDepth();
			}

			// TODO : make it so terrain & ocean cast shadows
			//glm::mat4 identity = glm::mat4(1);
			//GL(glUniformMatrix4fv(shader.getUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(identity)));
			//if (m_renderingOptions.m_renderTerrain)
			//	m_scene->m_terrain->renderDepth();
			//if (m_renderingOptions.m_renderOcean)
			//	m_scene->m_ocean->renderDepth();
		}

		shadowfb->m_shadowMap = savedShadowMap;
		GL(glCullFace(GL_BACK));
		clearBindings();

		// Revert for the scene.
		GL(glBindFramebuffer(GL_FRAMEBUFFER, fb->m_fbo));
		GL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
		GL(glViewport(0, 0, 640, 480));
	}

	void Renderer::renderPickedBoundingVolume()
	{
		auto& fb = m_scene->m_frameBuffer;
		auto& objList = m_scene->m_objectList;

		if (m_debugOptions.m_pickedObj > 0)
		{
			auto& shader = m_shaderManager.getShader("BoundingBox");
			shader.bind();

			//Update model matrix and draw if in frustum
			if (objList[m_debugOptions.m_pickedObj - 1].isInFrustum(m_camera) != OUTSIDE)
			{
				objList[m_debugOptions.m_pickedObj - 1].m_mesh->updateAABB(objList[m_debugOptions.m_pickedObj - 1].m_objectMatrix);
				glUniformMatrix4fv(shader.getUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(objList[m_debugOptions.m_pickedObj - 1].m_mesh->m_AABB.m_transform));
				objList[m_debugOptions.m_pickedObj - 1].renderBV();
			}
		}
	}

	void Renderer::renderAxis()
	{
		auto& fb = m_scene->m_frameBuffer;
		auto& objList = m_scene->m_objectList;

		Axis axis{};
		auto& shader = m_shaderManager.getShader("Axis");
		shader.bind();

		for (auto& obj : objList)
		{
			//Update model matrix and draw if in frustum
			//if (obj.isInFrustum(camera) != OUTSIDE)
			{
				glUniformMatrix4fv(shader.getUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(obj.m_objectMatrix));
				axis.render();
			}
		}
	}

	void Renderer::finalize()
	{
		GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		GL(glDepthMask(GL_TRUE));
		GL(glEnable(GL_DEPTH_TEST));
		auto& shader = m_shaderManager.getShader("Quad");
		shader.bind();
		GL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
		GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		GL(glViewport(0, 0, 640, 480));
		drawQuad();
	}

	void Renderer::renderImGui()
	{
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), translateDebugMode(m_debugOptions.m_debugMode).c_str());

		if (ImGui::CollapsingHeader("Rendering"))
		{
			ImGui::Checkbox("PostFX", &(m_renderingOptions.m_renderPostFx)); ImGui::SameLine(100);
			ImGui::Checkbox("Shadow", &(m_renderingOptions.m_renderShadow));
			ImGui::Checkbox("Ocean", &(m_renderingOptions.m_renderOcean)); ImGui::SameLine(100);
			ImGui::Checkbox("Grass", &(m_renderingOptions.m_renderGrass));
			ImGui::Checkbox("Terrain", &(m_renderingOptions.m_renderTerrain)); ImGui::SameLine(100);
			ImGui::Checkbox("SSAO", &(m_renderingOptions.m_renderSSAO));
		}

		if (ImGui::CollapsingHeader("Profiling"))
		{
			ImGui::RadioButton("CPU", &s_profileType, 0);
			ImGui::SameLine();
			ImGui::RadioButton("GPU", &s_profileType, 1);

			char buf[32];
			sprintf_s(buf, "%3.1f", m_timerValues.m_frame);
			ImGui::LabelText("Frame", buf);

			sprintf_s(buf, "  %3.1f", m_timerValues.m_shadow);
			ImGui::LabelText("Shadow", buf);

			sprintf_s(buf, "  %3.1f", m_timerValues.m_uboUpdate);
			ImGui::LabelText("UBO update", buf);

			sprintf_s(buf, "  %3.1f", m_timerValues.m_opaque);
			ImGui::LabelText("Opaque", buf);

			sprintf_s(buf, "    %3.1f", m_timerValues.m_geometry);
			ImGui::LabelText("Geometry", buf);

			sprintf_s(buf, "      %3.1f", m_timerValues.m_decal);
			ImGui::LabelText("Decal", buf);

			sprintf_s(buf, "    %3.1f", m_timerValues.m_ssao);
			ImGui::LabelText("SAO", buf);

			sprintf_s(buf, "    %3.1f", m_timerValues.m_light);
			ImGui::LabelText("Light", buf);

			sprintf_s(buf, "      %3.1f", m_timerValues.m_areaLight);
			ImGui::LabelText("AreaLight", buf);

			sprintf_s(buf, "    %3.1f", m_timerValues.m_resolve);
			ImGui::LabelText("Resolve", buf);

			sprintf_s(buf, "    %3.1f", m_timerValues.m_physicalAL);
			ImGui::LabelText("PhysicalAL", buf);

			sprintf_s(buf, "  %3.1f", m_timerValues.m_postFX);
			ImGui::LabelText("PostFx", buf);

			sprintf_s(buf, "    %3.1f", m_timerValues.m_tonemap);
			ImGui::LabelText("Tonemap", buf);

			sprintf_s(buf, "    %3.1f", m_timerValues.m_fxaa);
			ImGui::LabelText("FXAA", buf);
		}

		if (ImGui::CollapsingHeader("Scene"))
		{
			auto& sceneOptions = m_scene->m_sceneOptions;
			drawSlider("Hour", &(sceneOptions.m_hour), 6.f, 19.99f);
			drawSlider("Wetness", &(sceneOptions.m_wetness), 0.f, 1.f);
			drawSlider("CameraSpeed", &m_camera.m_cameraSpeed, 1.f, 50.f);
			drawSlider("Exposure", &(sceneOptions.m_exposure), 0.01f, 10.f);
		}

		if (ImGui::CollapsingHeader("AreaLight"))
		{
			ImGui::ColorEdit3("Color", (f32*)&(m_areaLightOptions.m_color));
			drawSlider("Intensity", &(m_areaLightOptions.m_intensity), 1.f, 100.f);
		}

		if (ImGui::CollapsingHeader("TAA"))
		{
			drawSlider("JitterSigma", &(m_taaOptions.m_jitterAASigma), 0.0f, 0.3f);
		}

		if (ImGui::CollapsingHeader("SAO"))
		{
			drawSlider("Radius", &(m_saoOptions.m_radius), 0.1f, 2.0f);
			drawSlider("Bias", &(m_saoOptions.m_bias), 0.01f, 0.2f);
			drawSlider("Intensity", &(m_saoOptions.m_intensity), 1.f, 5.f);
		}
	}

	void Renderer::renderSkybox()
	{
		auto& skyboxId = m_scene->m_IBLData.m_envCubeMapId;
		GL(glEnable(GL_DEPTH_TEST));
		GL(glDepthFunc(GL_EQUAL));
		auto& shader = m_shaderManager.getShader("Skybox");
		shader.bind();
		m_textureManager.bindTexture(skyboxId, GL_TEXTURE0);
		//glm::mat4 rot = glm::rotate(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
		//glUniformMatrix4fv(shader.getUniformLocation("rotation"), 1, GL_FALSE, glm::value_ptr(rot));
		drawQuad();
		GL(glDepthFunc(GL_LEQUAL));
		GL(glDisable(GL_DEPTH_TEST));
	}

	void Renderer::prepareSnow()
	{
		auto& snowfb = m_scene->m_snowFrameBuffer;
		auto& objList = m_scene->m_objectList;
		auto& fb = m_scene->m_frameBuffer;

		// Prepare shadow
		glBindFramebuffer(GL_FRAMEBUFFER, snowfb->m_fbo);
		glViewport(0, 0, 256, 256);

		{
			Timer timer{ m_timerValues.m_heightSnow };
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDepthMask(GL_TRUE);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_DEPTH_BUFFER_BIT);
			glDepthFunc(GL_LEQUAL);

			snowfb->bindForSnowPass(m_textureManager);
			auto& shader = m_shaderManager.getShader("Snow");
			shader.bind();

			glm::mat4 captureProjection = glm::ortho(-5.f, 5.f, -5.f, 5.f, 0.01f, 0.10f);
			glm::mat4 captureView = glm::lookAt(glm::vec3(0.0f, 0.01f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.01f));
			glm::mat4 transform = captureProjection * captureView;

			glUniformMatrix4fv(shader.getUniformLocation("gMVP"), 1, GL_FALSE, glm::value_ptr(transform));
			for (auto& obj : objList)
			{
				glUniformMatrix4fv(shader.getUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(obj.m_objectMatrix));
				obj.renderDepth();
			}
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
		}

		// Blur the heightmap
		{
			Timer timer{ m_timerValues.m_blurSnow };
			snowfb->bindForBlurPass(m_textureManager);
			auto& shader = m_shaderManager.getShader("Blur");
			shader.bind();
			drawQuad();
		}

		// Revert for the scene.
		glBindFramebuffer(GL_FRAMEBUFFER, fb->m_fbo);
		glViewport(0, 0, 640, 480);
	}

	void Renderer::renderPostEffects()
	{
		auto& fb = m_scene->m_frameBuffer;
		m_postEffectManager.renderAll(m_shaderManager, m_textureManager, *fb, m_timerValues, m_scene->m_sceneOptions);
	}

	void Renderer::renderAreaLight()
	{
		auto& areaLightList = m_scene->m_areaLightList;
		GL(glEnable(GL_DEPTH_TEST));
		GL(glDepthMask(GL_TRUE));
		auto& shader = m_shaderManager.getShader("AreaLight");
		shader.bind();

		for (auto& al : areaLightList)
		{
			GL(glUniformMatrix4fv(shader.getUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(al.m_transform)));
			GL(glUniform1fv(shader.getUniformLocation("intensity"), 1, &(m_areaLightOptions.m_intensity)));
			GL(glUniform3fv(shader.getUniformLocation("color"), 1, &(m_areaLightOptions.m_color[0])));
			m_textureManager.bindTexture(al.m_lightTextureId, GL_TEXTURE0);
			al.m_quad.render();
		}
		GL(glDisable(GL_DEPTH_TEST));
		GL(glDepthMask(GL_FALSE));
	}

	void Renderer::chooseDebugMode()
	{
		auto& fb = m_scene->m_frameBuffer;

		switch (m_debugOptions.m_debugMode)
		{
		case 0: // No debug
			m_textureManager.bindTexture(fb->m_finalTextureWithHudId, GL_TEXTURE0);
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, m_textureManager.getTextureHandle(fb->m_finalTextureWithHudId), 0));
			break;
		case 1: // Normal
			m_textureManager.bindTexture(fb->m_texturesId[FrameBuffer::GBUFFER_RT1], GL_TEXTURE0);
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, m_textureManager.getTextureHandle(fb->m_texturesId[FrameBuffer::GBUFFER_RT1]), 0));
			break;
		case 2: // AO
			m_textureManager.bindTexture(fb->m_aoTextureId, GL_TEXTURE0);
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, m_textureManager.getTextureHandle(fb->m_aoTextureId), 0));
			break;
		case 3: // AOBlurred
			m_textureManager.bindTexture(fb->m_aoTextureBlurred2Id, GL_TEXTURE0);
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, m_textureManager.getTextureHandle(fb->m_aoTextureBlurred2Id), 0));
			break;
		case 4: // SM
			m_textureManager.bindTexture(m_lightManager.m_shadowMap[0], GL_TEXTURE0);
			//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, m_textureManager.getTextureHandle(m_lightManager.m_shadowMap[0]), 0);
			break;
		case 5: // Diffuse light
			m_textureManager.bindTexture(fb->m_diffuseTextureId, GL_TEXTURE0);
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, m_textureManager.getTextureHandle(fb->m_diffuseTextureId), 0));
			break;
		case 6: // Specular light
			m_textureManager.bindTexture(fb->m_specularTextureId, GL_TEXTURE0);
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, m_textureManager.getTextureHandle(fb->m_specularTextureId), 0));
			break;
		case 7: // SSR
			m_textureManager.bindTexture(fb->m_ssrTextureId, GL_TEXTURE0);
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, m_textureManager.getTextureHandle(fb->m_ssrTextureId), 0));
			break;
		case 8: // Position
			m_textureManager.bindTexture(fb->m_texturesId[2], GL_TEXTURE0);
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, m_textureManager.getTextureHandle(fb->m_texturesId[2]), 0));
			break;
		case 9: // Albedo
			m_textureManager.bindTexture(fb->m_texturesId[0], GL_TEXTURE0);
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, m_textureManager.getTextureHandle(fb->m_texturesId[0]), 0));
			break;
		case 10: // Current frame
			m_textureManager.bindTexture(fb->m_finalTextureCurrentId, GL_TEXTURE0);
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, m_textureManager.getTextureHandle(fb->m_finalTextureCurrentId), 0));
			break;
		case 11: // Last frame
			m_textureManager.bindTexture(fb->m_finalTextureLastId, GL_TEXTURE0);
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, m_textureManager.getTextureHandle(fb->m_finalTextureLastId), 0));
			break;
		case 12: // Snow Displacement
			//m_textureManager.bindTexture(objList[1].m_mesh->m_heightMapsId[1], GL_TEXTURE0);
			//m_textureManager.bindTexture(shadowfb->m_depthTextureId, GL_TEXTURE0);
			break;
		}
	}

	// Adapted from https://learnopengl.com/#!PBR/IBL/Diffuse-irradiance
	// TODO : hardcoded atm, make it so we can change at runtime
	void Renderer::initIBL()
	{
		auto& fb = m_scene->m_frameBuffer;

		// Load envmap
		u32 envMapHdrId = m_textureManager.createTexture(HDR_EQUIRECTANGLE, "asset/GrandCanyon/GCanyon_C_YumaPoint_3k.hdr");

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		u32 captureFBO;
		u32 captureRBO;
		glGenFramebuffers(1, &captureFBO);
		glGenRenderbuffers(1, &captureRBO);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

		m_scene->m_IBLData.m_envCubeMapId = m_textureManager.createTexture(FBO_CUBEMAP_512);
		m_scene->m_IBLData.m_irrCubeMapId = m_textureManager.createTexture(FBO_HDR_CUBEMAP_32);
		m_scene->m_IBLData.m_prefilterMapId = m_textureManager.createTexture(FBO_HDR_CUBEMAP_128);
		m_scene->m_IBLData.m_brdfLUTId = m_textureManager.createTexture(FBO_TEXTURE2D_512_2CH);

		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
		glm::mat4 captureViews[] =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		auto& shader = m_shaderManager.getShader("EquirectangleToCubemap");
		shader.bind();
		m_textureManager.bindTexture(envMapHdrId, GL_TEXTURE0);
		glViewport(0, 0, 512, 512);
		glUniformMatrix4fv(shader.getUniformLocation("p"), 1, GL_FALSE, glm::value_ptr(captureProjection));

		for (u32 i = 0; i < 6; ++i)
		{
			glUniformMatrix4fv(shader.getUniformLocation("v"), 1, GL_FALSE, glm::value_ptr(captureViews[i]));
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_textureManager.getTextureHandle(m_scene->m_IBLData.m_envCubeMapId), 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			drawCube();
		}

		// Create an irradiance map by solving the diffuse integral by convolution
		glViewport(0, 0, 32, 32);
		auto& irradianceShader = m_shaderManager.getShader("Irradiance");
		irradianceShader.bind();
		m_textureManager.bindTexture(m_scene->m_IBLData.m_envCubeMapId, GL_TEXTURE0);
		glUniformMatrix4fv(irradianceShader.getUniformLocation("p"), 1, GL_FALSE, glm::value_ptr(captureProjection));

		for (u32 i = 0; i < 6; ++i)
		{
			glUniformMatrix4fv(irradianceShader.getUniformLocation("v"), 1, GL_FALSE, glm::value_ptr(captureViews[i]));
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_textureManager.getTextureHandle(m_scene->m_IBLData.m_irrCubeMapId), 0);
			GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
			GL(drawCube());
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
		auto& prefilterShader = m_shaderManager.getShader("Prefilter");
		prefilterShader.bind();
		m_textureManager.bindTexture(m_scene->m_IBLData.m_envCubeMapId, GL_TEXTURE0);
		glUniformMatrix4fv(prefilterShader.getUniformLocation("p"), 1, GL_FALSE, glm::value_ptr(captureProjection));

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		u32 maxMipLevels = 5;
		for (u32 mip = 0; mip < maxMipLevels; ++mip)
		{
			// Resize framebuffer according to mip-level size.
			u32 mipWidth = 128 * std::pow(0.5, mip);
			u32 mipHeight = 128 * std::pow(0.5, mip);
			glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
			glViewport(0, 0, mipWidth, mipHeight);

			f32 roughness = (f32)mip / (f32)(maxMipLevels - 1);
			glUniform1fv(prefilterShader.getUniformLocation("roughness"), 1, &roughness);

			for (u32 i = 0; i < 6; ++i)
			{
				glUniformMatrix4fv(prefilterShader.getUniformLocation("v"), 1, GL_FALSE, glm::value_ptr(captureViews[i]));
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_textureManager.getTextureHandle(m_scene->m_IBLData.m_prefilterMapId), mip);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				drawCube();
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Reconfigure capture framebuffer object and render screen-space quad with BRDF shader.
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureManager.getTextureHandle(m_scene->m_IBLData.m_brdfLUTId), 0);

		glViewport(0, 0, 512, 512);
		auto& brdfShader = m_shaderManager.getShader("BrdfShader");
		brdfShader.bind();
		m_textureManager.bindTexture(m_scene->m_IBLData.m_envCubeMapId, GL_TEXTURE0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawQuad();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}