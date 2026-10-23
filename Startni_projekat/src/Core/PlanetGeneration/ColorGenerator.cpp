#include "ColorGenerator.h"

namespace
{
	const int kTextureResolution = 100;
	const GLuint kTextureSlot = 1;
	const int kMaxBioms = 8;
}

ColorGenerator::ColorGenerator()
{
	ResourceManager& rm = ResourceManager::GetInstance();
	texture = rm.GetOrLoadGradientTexture("Gradijentna mapa", kTextureResolution * 2, kTextureSlot);
	UpdateColors();
}

ColorSettings& ColorGenerator::GetSettings()
{
	return settings;
}

void ColorGenerator::UpdateElevation(const glm::vec2& minMaxi)
{
	this->minMax = minMaxi;
}

void ColorGenerator::UpdateColors()
{
	const auto& biomes = settings.biomeSettings.biomes;
	const int numBiomes = static_cast<int>(biomes.size());
	if (numBiomes == 0)
		return;

	std::vector<glm::vec3> colors(static_cast<size_t>(kTextureResolution * 2) * numBiomes);

	int index = 0;
	const Gradient sortedOcean = settings.oceanColor.Sorted();
	for (const Biome& biome : biomes)
	{
		const Gradient sorted = biome.gradient.Sorted();
		for (int i = 0; i < kTextureResolution * 2; i++)
		{
			glm::vec3 gradientColor;
			if (i < kTextureResolution)
				gradientColor = sortedOcean.Evaluate(i / (kTextureResolution - 1.0f));
			else
				gradientColor = sorted.Evaluate((i - kTextureResolution) / (kTextureResolution - 1.0f));
			colors[index++] = glm::mix(gradientColor, biome.tint, biome.tintPercent);
		}
	}

	texture->SetColors(kTextureResolution, numBiomes, colors);
}

void ColorGenerator::SetShaderProgramParameters(const Shader& shader) const
{
	shader.setVec2("elevationMinMax", minMax);
	shader.setInt("colorGradient", kTextureSlot);

	const BiomeColorSettings& b = settings.biomeSettings;
	const int count = std::min(static_cast<int>(b.biomes.size()), kMaxBioms);

	float starts[kMaxBioms] = {};
	for (int i = 0; i < count; ++i)
		starts[i] = b.biomes[i].startHeight;

	shader.setInt("biomeCount", count);
	shader.setFloatArray("biomeStart", starts, kMaxBioms);
	shader.setFloat("biomeBlendRange", b.blendAmount / 2.0f + 0.001f);
	shader.setFloat("biomeNoiseOffset", b.noiseOffset);
	shader.setFloat("biomeNoiseStrength", b.noiseStrength);

	texture->Bind();
}