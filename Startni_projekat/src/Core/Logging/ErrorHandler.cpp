#include "ErrorHandler.h"

#include <iostream>

void GLClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* funciton, const char* file, int line)
{
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		std::cout << "[OpenGL Error] (" << error << "): " << funciton << std::endl
			<< "File: " << file << std::endl
			<< "Line: " << line << std::endl;
		return false;
	}
	return true;
}
