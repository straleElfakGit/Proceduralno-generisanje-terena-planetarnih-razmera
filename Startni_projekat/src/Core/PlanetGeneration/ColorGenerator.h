#ifndef COLOR_GENERATOR_CLASS_H
#define COLOR_GENERATOR_CLASS_H

#include <memory>
#include <glm/glm.hpp>
#include <vector>

#include "Settings/ColorSettings.h"
#include "Settings/NoiseSettings.h"
#include "Textures/GradientTexture.h"
#include "shaderClass.h"
#include "ResourceManager/ResourceManager.h"
#include "TerrainHeightCalculator.h"

class ColorGenerator
{
private:
	ColorSettings settings;
	GradientTexture* texture;

	glm::vec2 minMax = glm::vec2(-1e-3f, 1e-3f);

public:
	ColorGenerator();

	ColorSettings& GetSettings();

	void UpdateElevation(const glm::vec2& minMax);
	void UpdateColors();
	void SetShaderProgramParameters(const Shader& shader) const;

	void SetGradientTexture(GradientTexture* gradTex) { texture = gradTex; }
};

#endif