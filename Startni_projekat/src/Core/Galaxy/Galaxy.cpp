#include "Galaxy.h"
#include <array>

namespace
{
	const GLuint kSkyboxSlot = 0;

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
		4, 6, 5,  4, 7, 6,
		0, 2, 3,  0, 1, 2,
		1, 6, 2,  1, 5, 6,
		0, 7, 4,  0, 3, 7,
		3, 6, 7,  3, 2, 6,
		0, 5, 1,  0, 4, 5
	};

	const GLsizei kIndexCount = sizeof(kIndices) / sizeof(kIndices[0]);
}

Galaxy::Galaxy(const Shader& shader)
	: shader(shader)
{
	cubeMap = std::make_unique<CubeMap>(
		std::array<const char*, 6>{
		"assets/Textures/Background/SpaceSkybox1/galaxy+X.png",
			"assets/Textures/Background/SpaceSkybox1/galaxy-X.png",
			"assets/Textures/Background/SpaceSkybox1/galaxy+Y.png",
			"assets/Textures/Background/SpaceSkybox1/galaxy-Y.png",
			"assets/Textures/Background/SpaceSkybox1/galaxy+Z.png",
			"assets/Textures/Background/SpaceSkybox1/galaxy-Z.png"

			/*"assets/Textures/Background/SpaceSkybox2/skybox_left.png",
			"assets/Textures/Background/SpaceSkybox2/skybox_right.png",
			"assets/Textures/Background/SpaceSkybox2/skybox_up.png",
			"assets/Textures/Background/SpaceSkybox2/skybox_down.png",
			"assets/Textures/Background/SpaceSkybox2/skybox_front.png",
			"assets/Textures/Background/SpaceSkybox2/skybox_back.png"*/
	},
		kSkyboxSlot);

	cubeMap->texUnit(shader, "skybox", kSkyboxSlot);

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

void Galaxy::Render(const glm::mat4& view, const glm::mat4& projection) const
{
	glDepthFunc(GL_LEQUAL);

	shader.Activate();

	const glm::mat4 rotationOnlyView = glm::mat4(glm::mat3(view));
	const glm::mat4 projview = projection * rotationOnlyView;

	shader.setMat4("projview", projview);

	cubeMap->Bind();
	vao->Bind();

	glDrawElements(GL_TRIANGLES, kIndexCount, GL_UNSIGNED_INT, 0);

	vao->Unbind();
	cubeMap->Unbind();

	glDepthFunc(GL_LESS);
}