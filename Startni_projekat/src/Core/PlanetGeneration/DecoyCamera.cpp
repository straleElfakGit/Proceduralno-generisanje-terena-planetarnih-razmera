#include "DecoyCamera.h"

namespace
{
    const GLfloat kVertices[] = {
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f
    };

    const GLuint kIndices[] = {
        4, 5, 6,   4, 6, 7,
        1, 0, 3,   1, 3, 2,
        5, 1, 2,   5, 2, 6,
        0, 4, 7,   0, 7, 3,
        7, 6, 2,   7, 2, 3,
        0, 1, 5,   0, 5, 4
    };

    const GLsizei kIndexCount = sizeof(kIndices) / sizeof(kIndices[0]);
}

DecoyCamera::DecoyCamera()
{
    LOG_FUNC();
    ResourceManager& rm = ResourceManager::GetInstance();
    shaderPtr = rm.GetOrLoadShader("SateliteShader", "assets/Shaders/sateliteShader.vert", "assets/Shaders/sateliteShader.frag");

    vao = std::make_unique<VAOLayout>();
    vao->Bind();

    vbo = std::make_unique<VBO<GLfloat>>(kVertices, sizeof(kVertices));
    ebo = std::make_unique<EBO<GLuint>>(kIndices, sizeof(kIndices));

    VertexBufferLayout layout;
    layout.Push<float>(3);
    vao->addBuffer(*vbo, layout);

    vao->Unbind();
    vbo->Unbind();
    ebo->Unbind();
}

glm::vec3 DecoyCamera::GetPosition() const
{
    float x = centerPoint.x + std::cos(angle) * orbitRadius;
    float z = centerPoint.z + std::sin(angle) * orbitRadius;
    float y = centerPoint.y + height;
    return glm::vec3(x, y, z);
}

glm::mat4 DecoyCamera::GetViewMatrix() const
{
    glm::vec3 pos = GetPosition();
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 dir = glm::normalize(centerPoint - pos);
    if (std::abs(glm::dot(dir, up)) > 0.99f)
    {
        up = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    return glm::lookAt(pos, centerPoint, up);
}

glm::mat4 DecoyCamera::GetProjectionMatrix(int width, int height) const
{
    float aspectRatio = (height > 0) ? static_cast<float>(width) / height : 1.0f;
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

void DecoyCamera::Update(float deltaTime)
{
    if (!active) 
        return;

    angle += orbitSpeed * deltaTime;
    angle = std::fmod(angle, glm::two_pi<float>());
    shaderPtr->Activate();
    shaderPtr->setFloat("cubeScale", cubeScale);
}

void DecoyCamera::RenderGui()
{
    ImGui::Checkbox("Enable Decoy Camera (Debug Culling)", &active);

    if (active)
    {
        ImGui::Indent();
        ImGui::SliderFloat("Decoy Orbit Speed", &orbitSpeed, -3.0f, 3.0f);
        ImGui::SliderFloat("Decoy Orbit Radius", &orbitRadius, 10.0f, 200.0f);
        ImGui::SliderFloat("Decoy Height", &height, -50.0f, 50.0f);
        ImGui::SliderFloat("Decoy FOV", &fov, 10.0f, 120.0f);
        ImGui::SliderFloat("Decoy Cube Scale", &cubeScale, 0.01f, 2.0f);

        if (ImGui::Button("Reset Decoy Angle"))
            angle = 0.0f;

        ImGui::Unindent();
    }
}

void DecoyCamera::Render(const glm::mat4 projMat, const glm::mat4& viewMat)
{
    shaderPtr->Activate();
    glm::mat4 projview = projMat * viewMat;
    shaderPtr->setMat4("projview", projview);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, GetPosition());
    model = glm::rotate(model, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
    shaderPtr->setMat4("model", model);

    vao->Bind();
    glDrawElements(GL_TRIANGLES, kIndexCount, GL_UNSIGNED_INT, 0);
    vao->Unbind();
}