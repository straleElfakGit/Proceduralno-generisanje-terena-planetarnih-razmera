#include "PlanetQuadTree.h"

namespace
{
	//Gribb-Hartmann
	void ExtractFrustumPlanes(const glm::mat4& projview, std::array<glm::vec4, 6>& planes)
	{
		auto row = [&](int i) {
			return glm::vec4(projview[0][i], projview[1][i], projview[2][i], projview[3][i]);
		};

		const glm::vec4 r0 = row(0);
		const glm::vec4 r1 = row(1);
		const glm::vec4 r2 = row(2);
		const glm::vec4 r3 = row(3);

		planes[0] = r3 + r0;
		planes[1] = r3 - r0;
		planes[2] = r3 + r1;
		planes[3] = r3 - r1;
		planes[4] = r3 + r2;
		planes[5] = r3 - r2;

		for (auto& p : planes)
		{
			const float len = glm::length(glm::vec3(p));
			if (len > 1e-8f)
				p /= len;
		}
	}

	const double halfPi = 1.5707963267948966;

	void CapBoundingSphere(const glm::dvec3& axis, double radius, double halfAngle, glm::dvec3& outC, double& outR)
	{
		if (halfAngle >= halfPi)
		{
			outC = glm::dvec3(0.0);
			outR = radius;
			return;
		}
		outC = axis * (radius * std::cos(halfAngle));
		outR = radius * std::sin(halfAngle);
	}

	void MergeSpheres(const glm::dvec3& cA, double rA, const glm::dvec3& cB, double rB, glm::dvec3& outC, double& outR)
	{
		const glm::dvec3 d = cB - cA;
		const double dist = glm::length(d);
		if (dist + rB <= rA) { 
			outC = cA; 
			outR = rA; 
			return; 
		}
		if (dist + rA <= rB) { 
			outC = cB; 
			outR = rB; 
			return; 
		}
		outR = 0.5 * (dist + rA + rB);
		outC = (dist > 1e-12) ? (cA + d * ((outR - rA) / dist)) : cA;
	}
}

PlanetQuadTree::PlanetQuadTree(const PlanetQuadTreeSettings& s)
{
	Configure(s);
}

void PlanetQuadTree::Configure(const PlanetQuadTreeSettings& s)
{
	settings = s;
	settings.maxLevel = glm::clamp(settings.maxLevel, 0, maxPlanetLods - 1);
	settings.nodeResolution = std::max(2, settings.nodeResolution - (settings.nodeResolution % 2));

	const PlanetNodeBounds finest = ComputeBounds(0, settings.maxLevel, 0, 0);
	const float baseDiameter = 2.0f * finest.radius;

	const float range0 = settings.lodRangeFactor * baseDiameter;
	for (int lod = 0; lod < GetLodCount(); lod++)
		ranges[lod] = range0 * static_cast<float>(1u << lod);

	for (int lod = 0; lod < GetLodCount(); lod++)
	{
		const float prevRange = (lod == 0) ? 0.0f : ranges[lod - 1];
		morphStart[lod] = glm::mix(prevRange, ranges[lod], settings.morphStartRatio);
		morphEnd[lod] = ranges[lod];
	}
}

glm::dvec3 PlanetQuadTree::FaceToSphere(int face, double u, double v)
{
	static const glm::dvec3 localUps[6] = {
		{ 1.0, 0.0, 0.0}, {-1.0, 0.0, 0.0},
		{ 0.0, 1.0, 0.0}, { 0.0,-1.0, 0.0},
		{ 0.0, 0.0, 1.0}, { 0.0, 0.0,-1.0},
	};

	const glm::dvec3 localUp = localUps[face];
	const glm::dvec3 axisU(localUp.y, localUp.z, localUp.x);
	const glm::dvec3 axisV = glm::cross(axisU, localUp);

	const glm::dvec3 p = localUp + u * axisU + v * axisV;
	const glm::dvec3 p2 = p * p;

	const double x = p.x * std::sqrt(std::max(0.0, 1.0 - (p2.y + p2.z) * 0.5 + (p2.y * p2.z) / 3.0));
	const double y = p.y * std::sqrt(std::max(0.0, 1.0 - (p2.x + p2.z) * 0.5 + (p2.x * p2.z) / 3.0));
	const double z = p.z * std::sqrt(std::max(0.0, 1.0 - (p2.x + p2.y) * 0.5 + (p2.x * p2.y) / 3.0));

	return glm::dvec3(x, y, z);
}

PlanetNodeBounds PlanetQuadTree::ComputeBounds(int face, int level, int x, int y) const
{
	const double step = 2.0 / static_cast<double>(1ull << level);
	const double u0 = -1.0 + x * step;
	const double v0 = -1.0 + y * step;
	const double u1 = u0 + step;
	const double v1 = v0 + step;
	const double uc = 0.5 * (u0 + u1);
	const double vc = 0.5 * (v0 + v1);

	const glm::dvec3 axis = FaceToSphere(face, uc, vc);

	const double us[8] = { u0, u1, u0, u1, uc, uc, u0, u1 };
	const double vs[8] = { v0, v0, v1, v1, v0, v1, vc, vc };
	double halfAngle = 0.0;
	for (int i = 0; i < 8; ++i)
	{
		const glm::dvec3 d = FaceToSphere(face, us[i], vs[i]);
		const double a = std::acos(glm::clamp(glm::dot(axis, d), -1.0, 1.0));
		halfAngle = std::max(halfAngle, a);
	}
	halfAngle = std::min(halfAngle * 1.01, 1.5707963267948966);

	const double R = static_cast<double>(settings.planetRadius);
	const double Rin = R;
	const double Rout = R * (1.0 + static_cast<double>(settings.heightMax));

	glm::dvec3 innerC, outerC, c;
	double innerR, outerR, r;
	CapBoundingSphere(axis, Rin, halfAngle, innerC, innerR);

	if (settings.heightMax > 0.0f)
	{
		CapBoundingSphere(axis, Rout, halfAngle, outerC, outerR);
		MergeSpheres(innerC, innerR, outerC, outerR, c, r);
	}
	else
	{
		c = innerC;
		r = innerR;
	}

	PlanetNodeBounds b;
	b.center = glm::vec3(c);
	b.radius = static_cast<float>(r);
	return b;
}

bool PlanetQuadTree::IntersectsRange(const PlanetNodeBounds& b, int lod) const
{
	const float r = ranges[lod] + b.radius;
	return glm::distance(camera, b.center) <= r;
}

PlanetQuadTree::FrustumResult PlanetQuadTree::TestFrustum(const PlanetNodeBounds& b) const
{
	bool intersects = false;
	for (const auto& p : planes)
	{
		const float d = glm::dot(glm::vec3(p), b.center) + p.w;
		if (d < -b.radius)
			return FrustumResult::Outside;
		if (d < b.radius)
			intersects = true;
	}
	return intersects ? FrustumResult::Intersect : FrustumResult::Inside;
}

bool PlanetQuadTree::HorizonCulled(const PlanetNodeBounds& b) const
{
	const double R = static_cast<double>(settings.planetRadius);
	const double D = glm::length(glm::dvec3(camera));
	if (D <= R)
		return false;

	const glm::dvec3 c(b.center);
	const double cLen = glm::length(c);
	if (cLen < 1e-9)
		return false;

	const double rhoMax = cLen + static_cast<double>(b.radius);
	if (rhoMax <= R)
		return false; 

	const double delta = std::asin(glm::clamp(static_cast<double>(b.radius) / cLen, 0.0, 1.0));

	const glm::dvec3 camDir = glm::dvec3(camera) / D;
	const glm::dvec3 nodeDir = c / cLen;
	const double cosTheta = glm::clamp(glm::dot(camDir, nodeDir), -1.0, 1.0);
	const double theta = std::acos(cosTheta);

	const double horizonCam = std::acos(glm::clamp(R / D, -1.0, 1.0));
	const double horizonNode = std::acos(glm::clamp(R / rhoMax, -1.0, 1.0));

	return (theta - delta) > (horizonCam + horizonNode);
}

void PlanetQuadTree::Select(const PlanetView& view)
{
	camera = view.cameraInLocalPlanetSpace;
	ExtractFrustumPlanes(view.localToClip, planes);

	selection.clear();
	stats = Stats{};

	for (int face = 0; face < 6; face++)
		SelectNode(face, 0, 0, 0, false);
}

void PlanetQuadTree::SelectNode(int face, int level, int x, int y, bool parentInsideFrustum)
{
	stats.visited++;

	const int lod = settings.maxLevel - level;
	const PlanetNodeBounds bounds = ComputeBounds(face, level, x, y);

	if (settings.horizonCulling && HorizonCulled(bounds))
	{
		stats.culledHorizon++;
		return;
	}

	FrustumResult fr = parentInsideFrustum ? FrustumResult::Inside : TestFrustum(bounds);
	if (settings.frustumCulling && fr == FrustumResult::Outside)
	{
		stats.culledFrustum++;
		return;
	}
	if (!settings.frustumCulling)
		fr = FrustumResult::Intersect;

	const bool childInsideFrustum = (fr == FrustumResult::Inside);
	const bool needsSplit = (level < settings.maxLevel) && IntersectsRange(bounds, lod - 1);

	if (!needsSplit)
	{
		if (selection.size() >= settings.maxSelectedNodes)
		{
			stats.overflow = true;
			return;
		}
		selection.push_back(PlanetNode{ face, lod, x, y });
		stats.selected++;
		if (lod >= 0 && lod < maxPlanetLods)
			stats.perLod[lod]++;
		return;
	}

	const int cx = x * 2;
	const int cy = y * 2;
	SelectNode(face, level + 1, cx, cy, childInsideFrustum);
	SelectNode(face, level + 1, cx + 1, cy, childInsideFrustum);
	SelectNode(face, level + 1, cx, cy + 1, childInsideFrustum);
	SelectNode(face, level + 1, cx + 1, cy + 1, childInsideFrustum);
}