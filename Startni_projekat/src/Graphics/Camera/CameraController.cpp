#include "CameraController.h"

CameraController::CameraController(const CameraAltitudeSettings& s) : settings(s) { }

float CameraController::GetAltitude(const glm::vec3& cameraWorldPos) const
{
	if (planetInfluences.empty())
		return settings.maxSpeed / std::max(settings.speedPerUnitAltitude, 1e-6f);

	float best = std::numeric_limits<float>::max();
	for (const PlanetInfluence& influsence : planetInfluences)
	{
		const float centerDist = glm::length(cameraWorldPos - influsence.planetWorldPosition);
		const float surfaceDist = centerDist - (influsence.radius + influsence.maxTerrainHeight);
		best = std::min(best, surfaceDist);
	}
	return std::max(0.0f, best);
}

float CameraController::ComputeSpeed(const glm::vec3& cameraWorldPos) const
{
	const float altitude = GetAltitude(cameraWorldPos);
	const float speed = altitude * settings.speedPerUnitAltitude;
	return glm::clamp(speed, settings.minSpeed, settings.maxSpeed);
}

float CameraController::ComputeNearPlane(const glm::vec3& cameraWorldPos) const
{
	const float altitude = GetAltitude(cameraWorldPos);
	const float nearPlane = altitude * settings.nearPerUnitAltitude;
	return glm::clamp(nearPlane, settings.minNear, settings.maxNear);
}
