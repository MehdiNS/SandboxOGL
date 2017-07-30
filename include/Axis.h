#pragma once

namespace OGL
{
	class Axis
	{
	public:
		Axis();
		~Axis();
		void render();
	private:
		u32 vboV, vboC;
	};
}