#pragma once
#include "Texture.h"
#include "TextureManager.h"

class Cube
{
public:
	Cube();
	~Cube();

	void Render();

	GLuint vboV, vboC;

};

