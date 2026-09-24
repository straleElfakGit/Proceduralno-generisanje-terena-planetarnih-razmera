#ifndef GALAXY_CLASS_H
#define GALAXY_CLASS_H

#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Textures/CubeMap.h"
#include "Buffers/EBO.h"
#include "Buffers/VBO.h"
#include "Buffers/VAOLayout.h"
#include "shaderClass.h"

class Galaxy
{
private:
	const Shader& shader;

	std::unique_ptr<CubeMap> cubeMap;
	std::unique_ptr<VAOLayout> vao;
	std::unique_ptr<VBO<GLfloat>> vbo;
	std::unique_ptr<EBO<GLuint>> ebo;

public:
	explicit Galaxy(const Shader& shader);
	~Galaxy() = default;

	void Render(const glm::mat4& view, const glm::mat4& projection) const;
};

#endif
