#pragma once

namespace OGL
{
class Quad
{
public:
	Quad();
	~Quad();
	void render();

	u32 m_vbo;
	u32 m_vbot;
	u32 m_vao;
};
}