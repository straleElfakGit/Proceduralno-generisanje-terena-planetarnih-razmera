#include "Light.h"

Light::Light() :
	ambient(glm::vec3(0.0f)),
	diffuse(glm::vec3(1.0f)),
	specular(glm::vec3(1.0f)) { }

Light::Light(const glm::vec3 ambient, const glm::vec3 diffuse, const glm::vec3 specular) :
	ambient(ambient),
	diffuse(diffuse),
	specular(specular) { }
