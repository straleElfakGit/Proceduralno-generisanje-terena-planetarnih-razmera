#include "GLResource.h"

GLResource::GLResource() : ID(0) {}

GLResource::~GLResource() 
{
	Delete();
}