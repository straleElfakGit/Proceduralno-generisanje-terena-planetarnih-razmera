#include "SurfaceCamera.h"

float SurfaceCamera::ElevationAt(const glm::vec3& dir) const
{
	if (heightCalculator)
		return heightCalculator->ElevationAt(dir);
	return 1.0f + maxElevationFallback;
}

void SurfaceCamera::ApplyRotation(const glm::vec3& axis, float angle)
{
	surfaceDirection = glm::normalize(glm::rotate(surfaceDirection, angle, axis));
	lookDirection = glm::rotate(lookDirection, angle, axis);
}

bool SurfaceCamera::TryMove(const glm::vec3& tangentDir, float distance)
{
	if (distance <= 0.0f)
		return true;

	float len = glm::length(tangentDir);
	if (len < 1e-6f)
		return true;
	glm::vec3 dir = tangentDir / len;

	glm::vec3 axis = glm::normalize(glm::cross(surfaceDirection, dir));
	float angle = distance / planetRadius;

	glm::vec3 proposedDir = glm::normalize(glm::rotate(surfaceDirection, angle, axis));

	if (heightCalculator)
	{
		float hCur = ElevationAt(surfaceDirection) * planetRadius;
		float hNew = ElevationAt(proposedDir) * planetRadius;
		float slope = (hNew - hCur) / std::max(distance, 1e-4f);
		if (slope > maxClimbSlope)
			return false;
	}

	ApplyRotation(axis, angle);
	return true;
}

void SurfaceCamera::EnterFromWorld(const glm::vec3& worldPos, const glm::vec3& worldLookDir)
{
	float len = glm::length(worldPos);
	surfaceDirection = (len > 1e-6f) ? (worldPos / len) : glm::vec3(1.0f, 0.0f, 0.0f);

	glm::vec3 tangentLook = worldLookDir - surfaceDirection * glm::dot(worldLookDir, surfaceDirection);
	float tLen = glm::length(tangentLook);
	if (tLen > 1e-4f)
	{
		lookDirection = tangentLook / tLen;
	}
	else
	{
		glm::vec3 arbitrary = (std::abs(surfaceDirection.y) < 0.99f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
		lookDirection = glm::normalize(glm::cross(surfaceDirection, arbitrary));
	}

	active = true;
	firstClick = true;
}

void SurfaceCamera::MouseInput(GLFWwindow* window, int width, int height)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		if (firstClick)
		{
			glfwSetCursorPos(window, (width / 2), (height / 2));
			firstClick = false;
		}

		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
		float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

		glm::vec3 right = glm::normalize(glm::cross(lookDirection, surfaceDirection));
		glm::vec3 newLook = glm::rotate(lookDirection, glm::radians(-rotX), right);

		if (std::abs(glm::angle(newLook, surfaceDirection) - glm::radians(90.0f)) <= glm::radians(85.0f))
			lookDirection = newLook;

		lookDirection = glm::rotate(lookDirection, glm::radians(-rotY), surfaceDirection);

		glfwSetCursorPos(window, (width / 2), (height / 2));
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;
	}
}

void SurfaceCamera::Inputs(GLFWwindow* window, float deltaTime, int width, int height)
{
	const float moveSpeed = deltaTime * walkSpeed * boostMultiplier;

	glm::vec3 forwardTangent = lookDirection - surfaceDirection * glm::dot(lookDirection, surfaceDirection);
	float fLen = glm::length(forwardTangent);
	forwardTangent = (fLen > 1e-5f) ? (forwardTangent / fLen) : glm::vec3(0.0f);

	glm::vec3 rightTangent = glm::normalize(glm::cross(forwardTangent, surfaceDirection));

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) TryMove(forwardTangent, moveSpeed);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) TryMove(-forwardTangent, moveSpeed);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) TryMove(rightTangent, moveSpeed);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) TryMove(-rightTangent, moveSpeed);

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		boostMultiplier = 2.5f;
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
		boostMultiplier = 1.0f;

	MouseInput(window, width, height);

	surfaceDirection = glm::normalize(surfaceDirection);
	lookDirection = glm::normalize(lookDirection - surfaceDirection * glm::dot(lookDirection, surfaceDirection));
}

glm::vec3 SurfaceCamera::GetPosition() const
{
	float elevation = ElevationAt(surfaceDirection);
	float r = elevation * planetRadius + eyeHeight;
	return surfaceDirection * r;
}

glm::mat4 SurfaceCamera::GetViewMatrix() const
{
	glm::vec3 pos = GetPosition();
	return glm::lookAt(pos, pos + lookDirection, surfaceDirection);
}

glm::mat4 SurfaceCamera::GetProjectionMatrix(float fovDeg, float nearPlane, float farPlane, int width, int height) const
{
	return glm::perspective(glm::radians(fovDeg), (float)width / height, nearPlane, farPlane);
}

void SurfaceCamera::Matrix(float fovDeg, float nearPlane, float farPlane, Shader& shader, const char* uniform, int width, int height) const
{
	glm::mat4 view = GetViewMatrix();
	glm::mat4 projection = GetProjectionMatrix(fovDeg, nearPlane, farPlane, width, height);
	shader.setMatrix(uniform, projection * view);
}

void SurfaceCamera::SetPositionToShader(const std::string& uniform, const Shader& shaderProgram) const
{
	shaderProgram.setVec3(uniform, GetPosition());
}

