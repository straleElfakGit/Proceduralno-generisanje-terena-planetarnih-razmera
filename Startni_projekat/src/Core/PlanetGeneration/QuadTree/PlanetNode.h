#ifndef PLANET_NODE_H
#define PLANET_NODE_H

struct PlanetNode
{
	int face;
	int lod;
	int x;
	int y;
};
static_assert(sizeof(PlanetNode) == 16, "Velicina PlanetNode-a i raspored mora da odgovarju ivec4 u std430");

#endif // !PLANET_NODE_H
