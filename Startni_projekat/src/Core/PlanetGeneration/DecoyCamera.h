#ifndef DECOY_CAMERA_H
#define DECOY_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include "imgui/imgui.h"
#include "Buffers/VAOLayout.h"
#include "Buffers/EBO.h"
#include "Buffers/VBO.h"
#include "shaderClass.h"
#include "ResourceManager/ResourceManager.h"

class DecoyCamera
{
private:
	bool active = false;
    float orbitRadius = 30.0f;
    float orbitSpeed = 0.5f;
    float angle = 0.0f;
    float height = 0.0f;

    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    glm::vec3 centerPoint = glm::vec3(0.0f);

    float cubeScale = 0.2f;
    std::unique_ptr<VAOLayout> vao;
    std::unique_ptr<VBO<GLfloat>> vbo;
    std::unique_ptr<EBO<GLuint>> ebo;

    Shader* shaderPtr;

public:
    DecoyCamera();
    
    glm::vec3 GetPosition() const;
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(int width, int height) const;

    bool IsActive() const { return active; }
    void SetActive(bool state) { active = state; }
    float GetFOV() const { return fov; }

    void SetCenterPoint(const glm::vec3& center) { centerPoint = center; }
    void SetRadius(float r) { orbitRadius = r; }

    void Update(float deltaTime);
    void Render(const glm::mat4 projMat, const glm::mat4& viewMat);
    void RenderGui();
};

#endif // !DECOY_CAMERA_H
