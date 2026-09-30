#include "TexturePlanetScene.h"

namespace
{
	const std::vector<const char*> terrainPaths = {
		"assets/Textures/Terrain/dirt_01_diffuse-1024.png",
		"assets/Textures/Terrain/grass1-albedo3-1024.png",
		"assets/Textures/Terrain/sandyground-albedo-1024.png",
		"assets/Textures/Terrain/worn-bumpy-rock-albedo-1024.png",
		"assets/Textures/Terrain/rock-snow-ice-albedo-1024.png",
		"assets/Textures/Terrain/snow-packed-albedo-1024.png",
		"assets/Textures/Terrain/rough-wet-cobble-albedo-1024.png",
		"assets/Textures/Terrain/sandy-rocks1-albedo-1024.png"
	};

	const std::vector<const char*> normalPaths = {
		"assets/Textures/Terrain/dirt_01_normal-1024.jpg",
		"assets/Textures/Terrain/grass1-normal-1024r.png",
		"assets/Textures/Terrain/sandyground-normal-1024.jpg",
		"assets/Textures/Terrain/worn-bumpy-rock-normal-1024.jpg",
		"assets/Textures/Terrain/rock-snow-ice-normal-1024.jpg",
		"assets/Textures/Terrain/snow-packed-normal-1024.jpg",
		"assets/Textures/Terrain/rough-wet-cobble-normal-1024.jpg",
		"assets/Textures/Terrain/sandy-rocks1-normal-1024.jpg"
	};
}

TexturePlanetScene::TexturePlanetScene(ApplicationBase* app) : PlanetSceneBase(app)
{
	ResourceManager& rm = ResourceManager::GetInstance();
	shaderPtr = rm.GetOrLoadShader("TextureShader", "assets/Shaders/planetShader.vert", "assets/Shaders/planetShader.frag");

	textures[0] = rm.GetOrLoadTexture("Water", "assets/Textures/Terrain/water.jpg", GL_TEXTURE_2D, 2, GL_RGB, GL_UNSIGNED_BYTE);
	textures[1] = rm.GetOrLoadTexture("Send", "assets/Textures/Terrain/sand.jpg", GL_TEXTURE_2D, 3, GL_RGB, GL_UNSIGNED_BYTE);
	textures[2] = rm.GetOrLoadTexture("Grass", "assets/Textures/Terrain/grass.jpg", GL_TEXTURE_2D, 4, GL_RGB, GL_UNSIGNED_BYTE);
	textures[3] = rm.GetOrLoadTexture("StonyGrass", "assets/Textures/Terrain/stonyGrass.jpg", GL_TEXTURE_2D, 5, GL_RGB, GL_UNSIGNED_BYTE);
	textures[4] = rm.GetOrLoadTexture("Rock", "assets/Textures/Terrain/rocky.jpg", GL_TEXTURE_2D, 6, GL_RGB, GL_UNSIGNED_BYTE);
	textures[5] = rm.GetOrLoadTexture("Mountain", "assets/Textures/Terrain/mountains.jpg", GL_TEXTURE_2D, 7, GL_RGB, GL_UNSIGNED_BYTE);
	textures[6] = rm.GetOrLoadTexture("Snow", "assets/Textures/Terrain/snow.jpg", GL_TEXTURE_2D, 8, GL_RGB, GL_UNSIGNED_BYTE);

	textures[0]->texUnit(*shaderPtr, "texWater", 2);
	textures[1]->texUnit(*shaderPtr, "texSand", 3);
	textures[2]->texUnit(*shaderPtr, "texGrass", 4);
	textures[3]->texUnit(*shaderPtr, "texStonyGrass", 5);
	textures[3]->texUnit(*shaderPtr, "texRock", 6);
	textures[5]->texUnit(*shaderPtr, "texMountain", 7);
	textures[6]->texUnit(*shaderPtr, "texSnow", 8);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
}

TexturePlanetScene::~TexturePlanetScene() { }

void TexturePlanetScene::UpdateWaterPosition(float deltaTime)
{
	textureSettings.waterOffset += textureSettings.waterSpeed * deltaTime;
	textureSettings.waterOffset.x = std::fmod(textureSettings.waterOffset.x, 1.0f);
	textureSettings.waterOffset.y = std::fmod(textureSettings.waterOffset.y, 1.0f);

	shaderPtr->Activate();

	shaderPtr->setVec2("waterOffset", textureSettings.waterOffset);
}

void TexturePlanetScene::UpdateSpecific(float deltaTime)
{
	UpdateWaterPosition(deltaTime);
}

void TexturePlanetScene::RenderSpecific()
{
	for (int i = 0; i < 7; i++)
		textures[i]->Bind();

	if (planetSettings.showMesh)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindVertexArray(vao);
	int res = static_cast<int>(planetSettings.resolution);
	int numOfVertices = res * res * 6 * 6;
	glDrawArrays(GL_TRIANGLES, 0, numOfVertices);
	glBindVertexArray(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void TexturePlanetScene::RenderPlanetPropertiesGui()
{
	ImGui::Checkbox("Show Mesh", &planetSettings.showMesh);

	int currentRes = static_cast<int>(planetSettings.resolution);
	if (ImGui::SliderInt("Node resolution", &currentRes, 2, 256))
		planetSettings.resolution = static_cast<unsigned int>(currentRes);

	float currentRadius = planetSettings.radius;
	if (ImGui::SliderFloat("Radius", &currentRadius, 5.0f, 200.0f))
	{
		planetSettings.radius = currentRadius;
		LOG_INFO("Radius set to {}", planetSettings.radius);
	}
}

void TexturePlanetScene::RenderTextureSettings()
{
	ImGui::Text("Water Animation");
	ImGui::SliderFloat2("Water Speed", glm::value_ptr(textureSettings.waterSpeed), -0.1f, 0.1f);
}

void TexturePlanetScene::RenderGuiSpecific()
{
	if (ImGui::CollapsingHeader("Textures settings"))
		RenderTextureSettings();
}