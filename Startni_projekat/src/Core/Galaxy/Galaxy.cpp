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

Galaxy::Galaxy()
{
	ResourceManager& rm = ResourceManager::GetInstance();
	shaderPtr = rm.GetOrLoadShader("SkyboxShader", "assets/Shaders/skyBoxShader.vert", "assets/Shaders/skyBoxShader.frag");

	cubeMap = rm.GetOrLoadCubeMap("SpaceSkybox1",
		std::array<const char*, 6>{
		    /*"assets/Textures/Background/SpaceSkybox1/galaxy+X.png",
			"assets/Textures/Background/SpaceSkybox1/galaxy-X.png",
			"assets/Textures/Background/SpaceSkybox1/galaxy+Y.png",
			"assets/Textures/Background/SpaceSkybox1/galaxy-Y.png",
			"assets/Textures/Background/SpaceSkybox1/galaxy+Z.png",
			"assets/Textures/Background/SpaceSkybox1/galaxy-Z.png"*/

			/*"assets/Textures/Background/SpaceSkybox2/skybox_left.png",
			"assets/Textures/Background/SpaceSkybox2/skybox_right.png",
			"assets/Textures/Background/SpaceSkybox2/skybox_up.png",
			"assets/Textures/Background/SpaceSkybox2/skybox_down.png",
			"assets/Textures/Background/SpaceSkybox2/skybox_front.png",
			"assets/Textures/Background/SpaceSkybox2/skybox_back.png"*/

			
			"assets/Textures/Background/SpaceSkybox3/space-posx.jpg",
			"assets/Textures/Background/SpaceSkybox3/space-negx.jpg",
			"assets/Textures/Background/SpaceSkybox3/space-posy.jpg",
			"assets/Textures/Background/SpaceSkybox3/space-negy.jpg",
			"assets/Textures/Background/SpaceSkybox3/space-posz.jpg",
			"assets/Textures/Background/SpaceSkybox3/space-negz.jpg",
			
	},
		kSkyboxSlot);

	cubeMap->texUnit(*shaderPtr, "skybox", kSkyboxSlot);

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

	shaderPtr->Activate();

	const glm::mat4 rotationOnlyView = glm::mat4(glm::mat3(view));
	const glm::mat4 projview = projection * rotationOnlyView;

	shaderPtr->setMat4("projview", projview);

	cubeMap->Bind();
	vao->Bind();

	glDrawElements(GL_TRIANGLES, kIndexCount, GL_UNSIGNED_INT, 0);

	vao->Unbind();
	cubeMap->Unbind();

	glDepthFunc(GL_LESS);
}