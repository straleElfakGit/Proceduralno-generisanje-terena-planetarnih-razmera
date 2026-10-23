#ifndef TERRAIN_SHAPE_SETTINGS_H
#define TERRAIN_SHAPE_SETTINGS_H

#include <vector>

#include "NoiseSettings.h"

enum class NoiseFilterType
{
	Simple = 0,
	Rigid = 1
};

struct TerrainLayerSettings
{
	NoiseSettings noise;
	NoiseFilterType filterType = NoiseFilterType::Simple;
	float weightMultiplier = 1.0f;       
	bool enabled = true;
	bool useFirstLayerAsMask = false;
};

struct TerrainShapeSettings
{
	static const int maxLayers = 4;

	std::vector<TerrainLayerSettings> layers;

	TerrainShapeSettings()
	{
		layers.emplace_back();
	}

	bool CanAddLayer() const { return static_cast<int>(layers.size()) < maxLayers; }

	void AddRigidLayer(unsigned int seed)
	{
		if (!CanAddLayer())
			return;

		TerrainLayerSettings layer;
		layer.filterType = NoiseFilterType::Rigid;
		layer.useFirstLayerAsMask = true;
		layer.weightMultiplier = 0.8f;
		layer.noise.seed = seed;
		layer.noise.strength = 1.0f;
		layer.noise.baseRoughness = 1.5f;
		layer.noise.roughness = 2.2f;
		layer.noise.persistance = 0.5f;
		layer.noise.numberOfOctaves = 4;
		layer.noise.minValue = 0.5f;
		layers.push_back(layer);
	}
};



#endif //!TERRAIN_SHAPE_SETTINGS_H