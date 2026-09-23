#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>

#include"shaderClass.h"

class Camera
{
private:
	glm::vec3 Position;
	glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

	bool firstClick = true;

	float speed = 20.0f;
	float sensitivity = 100.0f;

	void MouseInput(GLFWwindow* window, int width, int height);

public:
	Camera(const glm::vec3& position);

	void Matrix(float FOVdeg, float nearPlane, float farPlane, Shader& shader, const char* uniform, int width, int height);
	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix(float FOVdeg, float nearPlane, float farPlane, int width, int height) const;
	
	void Inputs(GLFWwindow* window, float deltaTime, int width, int height);
	void SetPositionToShader(const std::string& uniform, const Shader& shaderProgram);

	glm::vec3 GetOrientation() const;
	glm::vec3 GetPosition() const;
};
#endif