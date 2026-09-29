#ifndef PLANET_QUADTREE_H
#define PLANET_QUADTREE_H

#include <vector>
#include <array>

#include "PlanetQuadTreeSettings.h"
#include "PlanetNode.h"

const int maxPlanetLods = 16;

struct Stats
{
	int visited = 0;
	int selected = 0;
	int culledFrustum = 0;
	int culledHorizon = 0;
	int rangeViolations = 0;  
	bool overflow = false; 
	std::array<int, maxPlanetLods> perLod{};
};

class PlanetQuadTree
{
private:
	enum class FrustumResult { Outside, Intersect, Inside };

	PlanetQuadTreeSettings settings;

	std::array<float, maxPlanetLods> ranges{};
	std::array<float, maxPlanetLods> morphStart{};
	std::array<float, maxPlanetLods> morphEnd{};

	glm::vec3 camera{ 0.0f };
	std::array<glm::vec4, 6> planes{};
	std::vector<PlanetNode> selection;
	Stats stats;

	void SelectNode(int face, int level, int x, int y, bool parentInsideFrustum);
	bool HorizonCulled(const PlanetNodeBounds& b) const;
	FrustumResult TestFrustum(const PlanetNodeBounds& b) const;
	bool IntersectsRange(const PlanetNodeBounds& b, int lod) const;

public:
	explicit PlanetQuadTree(const PlanetQuadTreeSettings& settings = PlanetQuadTreeSettings());

	void Configure(const PlanetQuadTreeSettings& settings);

	void Select(const PlanetView& view);

	const std::vector<PlanetNode>& GetSelection() const { return selection; }
	const PlanetQuadTreeSettings& GetSettings() const { return settings; }
	const Stats& GetStats() const { return stats; }

	const float* GetRanges() const { return ranges.data(); }
	const float* GetMorphStart() const { return morphStart.data(); }
	const float* GetMorphEnd() const { return morphEnd.data(); }
	int GetLodCount() const { return settings.maxLevel + 1; }

	static glm::dvec3 FaceToSphere(int face, double u, double v);

	PlanetNodeBounds ComputeBounds(int face, int level, int x, int y) const;
};

#endif // !PLANET_QUADTREE_H

