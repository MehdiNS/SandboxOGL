#include "stdafx.h"
#include "Skybox.h"
#include "Util.h"

namespace OGL
{
	static GLfloat points[] = {
		-10.0f,-10.0f,-10.0f, // triangle 1 : begin
		-10.0f,-10.0f, 10.0f,
		-10.0f, 10.0f, 10.0f, // triangle 1 : end
		10.0f, 10.0f,-10.0f, // triangle 2 : begin
		-10.0f,-10.0f,-10.0f,
		-10.0f, 10.0f,-10.0f, // triangle 2 : end
		10.0f,-10.0f, 10.0f,
		-10.0f,-10.0f,-10.0f,
		10.0f,-10.0f,-10.0f,
		10.0f, 10.0f,-10.0f,
		10.0f,-10.0f,-10.0f,
		-10.0f,-10.0f,-10.0f,
		-10.0f,-10.0f,-10.0f,
		-10.0f, 10.0f, 10.0f,
		-10.0f, 10.0f,-10.0f,
		10.0f,-10.0f, 10.0f,
		-10.0f,-10.0f, 10.0f,
		-10.0f,-10.0f,-10.0f,
		-10.0f, 10.0f, 10.0f,
		-10.0f,-10.0f, 10.0f,
		10.0f,-10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f,-10.0f,-10.0f,
		10.0f, 10.0f,-10.0f,
		10.0f,-10.0f,-10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f,-10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f, 10.0f,-10.0f,
		-10.0f, 10.0f,-10.0f,
		10.0f, 10.0f, 10.0f,
		-10.0f, 10.0f,-10.0f,
		-10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		-10.0f, 10.0f, 10.0f,
		10.0f,-10.0f, 10.0f
	};

	Skybox::Skybox(TextureManager& tm)
	{
		glGenBuffers(1, &skyboxVBO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);

		glGenVertexArrays(1, &skyboxVAO);
		glBindVertexArray(skyboxVAO);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		// Cubemap (Skybox)
		vector<std::string> faces;
		faces.push_back("skybox/posx.jpg");
		faces.push_back("skybox/negx.jpg");
		faces.push_back("skybox/posy.jpg");
		faces.push_back("skybox/negy.jpg");
		faces.push_back("skybox/posz.jpg");
		faces.push_back("skybox/negz.jpg");
		string s = "skybox";
		//tm.GetCubemap(s, faces);
	}

	Skybox::Skybox()
	{
	}

	Skybox::~Skybox()
	{
	}

	void Skybox::Render()
	{
		glBindVertexArray(skyboxVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}