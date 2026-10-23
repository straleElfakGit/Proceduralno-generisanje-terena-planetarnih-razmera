#ifndef SUN_CLASS_H
#define SUN_CLASS_H

#include <glad/glad.h>
#include <glm/glm.hpp>


#include "shaderClass.h"
#include "ResourceManager/ResourceManager.h"

struct SunVisualSettings
{
	bool visible = true;
	float angularRadiusDeg = 2.0f;  
	float glowScale = 8.0f;     
	float glowStrength = 0.6f;
	float coreIntensity = 2.5f;
	glm::vec3 color = glm::vec3(1.0f, 0.9f, 0.7f);
};

class Sun
{
private:
	Shader* shaderPtr;
	GLuint vao = 0;
	SunVisualSettings settings;

public:
	Sun();
	~Sun();

	SunVisualSettings& GetSettings() { return settings; }

	void Render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& directionToSun) const;
};


#endif // !SUN_CLASS_H