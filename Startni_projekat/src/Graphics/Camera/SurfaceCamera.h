#ifndef SURCE_CAMERA_H
#define SURCE_CAMERA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <algorithm>
#include <cmath>

#include "shaderClass.h"
#include "PlanetGeneration/TerrainHeightCalculator.h"

class SurfaceCamera
{
private:
	glm::vec3 surfaceDirection = glm::vec3(1.0f, 0.0f, 0.0f); // local-up vektor
	glm::vec3 lookDirection = glm::vec3(0.0f, 0.0f, -1.0f); // tangenta na sferu

	float planetRadius = 50.0f;
	float eyeHeight = 1.63f;
	float maxElevationFallback = 0.0f;

	const TerrainHeightCalculator* heightCalculator = nullptr;

	float walkSpeed = 4.0f;
	float boostMultiplier = 1.0f;
	float sensitivity = 100.0f;
	float maxClimbSlope = 1.5f;

	bool firstClick = true;
	bool active = false;

	void MouseInput(GLFWwindow* window, int width, int height);

	float ElevationAt(const glm::vec3& dir) const;
	void ApplyRotation(const glm::vec3& axis, float angle);
	bool TryMove(const glm::vec3& tangentDir, float distance);

public:
	void SetPlanetRadius(float r) { planetRadius = r; }
	void SetEyeHeight(float h) { eyeHeight = h; }
	void SetMaxElevationFallback(float m) { maxElevationFallback = m; maxElevationFallback = 0.0f; }
	void SetHeightCalculator(const TerrainHeightCalculator* calculator) { heightCalculator = calculator; }
	void SetMaxClimbSlope(float slope) { maxClimbSlope = slope; }
	void SetWalkSpeed(float s) { walkSpeed = s; }

	bool IsActive() const { return active; }
	void SetActive(bool a) { active = a; firstClick = true; }

	void EnterFromWorld(const glm::vec3& worldPos, const glm::vec3& worldLookDir);

	glm::vec3 GetExitPosition() const { return GetPosition(); }
	glm::vec3 GetExitOrientation() const { return lookDirection; }

	void Inputs(GLFWwindow* window, float deltaTime, int width, int height);

	glm::vec3 GetPosition() const;
	glm::vec3 GetOrientation() const { return lookDirection; }
	glm::vec3 GetUp() const { return surfaceDirection; }

	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix(float fovDeg, float nearPlane, float farPlane, int width, int height) const;
	void Matrix(float fovDeg, float nearPlane, float farPlane, Shader& shader, const char* uniform, int width, int height) const;
	void SetPositionToShader(const std::string& uniform, const Shader& shaderProgram) const;

};

#endif // !SURCE_CAMERA_H