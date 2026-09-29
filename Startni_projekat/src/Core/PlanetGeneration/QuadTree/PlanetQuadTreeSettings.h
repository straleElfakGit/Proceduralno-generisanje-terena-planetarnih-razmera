#ifndef PLANET_QUADTREE_SETTINGS
#define PLANET_QUADTREE_SETTINGS

#include <glm/glm.hpp>

struct PlanetQuadTreeSettings
{
	float planetRadius = 50.0f;
	int maxLevel = 9;
	int nodeResolution = 32;
	float lodRangeFactor = 6.0f;
	float morphStartRatio = 0.5f;
	float heightMax = 0.0f;

	bool frustumCulling = true;
	bool horizonCulling = true;

	unsigned long long maxSelectedNodes = 16384;
};

struct PlanetView
{
	glm::vec3 cameraInLocalPlanetSpace;
	glm::mat4 localToClip;
};

struct PlanetNodeBounds
{
	glm::vec3 center;
	float radius;
};

#endif // !PLANET_QUADTREE_SETTINGS
