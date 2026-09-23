#include "Material.h"

Material::Material(float shininess) : shininess(shininess) 
{
	if (shininess > 128.0f)
		this->shininess = 128.0f;
	else if (shininess < 1.0f)
		this->shininess = 1.0f;
}

Material::~Material()
{
	//Delete();
}