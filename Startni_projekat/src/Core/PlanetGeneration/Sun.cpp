#include "Sun.h"

#include <cmath>

Sun::Sun()
{
	ResourceManager& rm = ResourceManager::GetInstance();
	shaderPtr = rm.GetOrLoadShader("SunShader", "assets/Shaders/sunShader.vert", "assets/Shaders/sunShader.frag");

	glGenVertexArrays(1, &vao);
}

Sun::~Sun()
{
	if (vao != 0)
		glDeleteVertexArrays(1, &vao);
}

void Sun::Render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& directionToSun) const
{
	if (!settings.visible)
		return;

	shaderPtr->Activate();

	const glm::mat4 viewRot = glm::mat4(glm::mat3(view));
	shaderPtr->setMat4("viewRot", viewRot);
	shaderPtr->setMat4("projection", projection);
	shaderPtr->setVec3("sunDir", glm::normalize(directionToSun));

	const float angular = glm::radians(settings.angularRadiusDeg);
	shaderPtr->setFloat("quadHalfSize", std::tan(angular) * settings.glowScale);
	shaderPtr->setFloat("glowScale", settings.glowScale);
	shaderPtr->setFloat("glowStrength", settings.glowStrength);
	shaderPtr->setFloat("coreIntensity", settings.coreIntensity);
	shaderPtr->setVec3("sunColor", settings.color);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}
