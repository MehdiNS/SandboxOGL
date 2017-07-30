#include "stdafx.h"
#include "Axis.h"
#include "Util.h"

namespace OGL
{
	static float vertices[] = {
		0.0f, 0.0f, 0.0f,// X
		10.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,// Y
		0.0f, 10.0f, 0.0f,
		0.0f, 0.0f, 0.0f, //Z
		0.0f, 0.0f, 10.0f
	};

	static float colors[] = {
		1.0f, 0.0f, 0.0f,// X
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,// Y
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, //Z
		0.0f, 0.0f, 1.0f
	};

	Axis::Axis()
	{
		glGenBuffers(1, &vboV);
		glBindBuffer(GL_ARRAY_BUFFER, vboV);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &vboC);
		glBindBuffer(GL_ARRAY_BUFFER, vboC);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}


	Axis::~Axis()
	{
	}

	void Axis::render() {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vboV);
		glVertexAttribPointer(
			0,                  
			3,                  
			GL_FLOAT,           
			GL_FALSE,           
			0,                  
			(void*)0            
			);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, vboC);
		glVertexAttribPointer(
			1,                                
			3,                                
			GL_FLOAT,                         
			GL_FALSE,                         
			0,                                
			(void*)0                          
			);

		glDrawArrays(GL_LINES, 0, 9);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}