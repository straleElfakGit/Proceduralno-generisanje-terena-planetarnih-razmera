#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <glm/glm.hpp>
#include <vector>
#include <algorithm>
#include <limits>

struct PlanetInfluence
{
	glm::vec3 planetWorldPosition = glm::vec3(0.0f);
	float radius = 1.0f;
	float maxTerrainHeight = 0.0f;
};

struct CameraAltitudeSettings
{
	float speedPerUnitAltitude = 0.5f;
	float minSpeed = 0.25f;
	float maxSpeed = 500.0f;

	float nearPerUnitAltitude = 0.02f;
	float minNear = 0.05f;
	float maxNear = 5.0f;
};

class CameraController
{
private:
	CameraAltitudeSettings settings;
	std::vector<PlanetInfluence> planetInfluences;

public:
	explicit CameraController(const CameraAltitudeSettings& s = CameraAltitudeSettings());

	void SetSettings(const CameraAltitudeSettings& s) { settings = s; }
	CameraAltitudeSettings& GetSettingsRef() { return settings; }
	const CameraAltitudeSettings& GetSettings() const { return settings; }

	void SetInfluences(std::vector<PlanetInfluence> newInfluences) { planetInfluences = std::move(newInfluences); }
	void SetSingleInfluence(const PlanetInfluence& inf) { planetInfluences.assign(1, inf); }

	float GetAltitude(const glm::vec3& cameraWorldPos) const;

	float ComputeSpeed(const glm::vec3& cameraWorldPos) const;
	float ComputeNearPlane(const glm::vec3& cameraWorldPos) const;

};

#endif // !CAMERA_CONTROLLER_H
