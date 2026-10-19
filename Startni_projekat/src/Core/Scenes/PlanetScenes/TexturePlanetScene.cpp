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

	waterTexture = rm.GetOrLoadTexture("Water", "assets/Textures/Terrain/water.jpg", GL_TEXTURE_2D, waterUnit, GL_RGB, GL_UNSIGNED_BYTE);
	waterTexture->SetFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	noiseTexture = rm.GetOrLoadTexture("BombNoise", "assets/Textures/Terrain/simplex-noise.png", GL_TEXTURE_2D, noiseUnit, GL_RGBA, GL_UNSIGNED_BYTE);
	noiseTexture->SetFilter(GL_LINEAR, GL_LINEAR);

	diffuseArray = rm.GetOrLoadTextureArray("DiffuseArray", terrainPaths, diffuseUnit, true);
	normalArray = rm.GetOrLoadTextureArray("NormalArray", normalPaths, normalUnit, false);

	waterTexture->texUnit(*shaderPtr, "texWater", waterUnit);
	noiseTexture->texUnit(*shaderPtr, "noiseMap", noiseUnit);
	diffuseArray->texUnit(*shaderPtr, "diffuseMap", diffuseUnit);
	normalArray->texUnit(*shaderPtr, "normalMap", normalUnit);

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

void TexturePlanetScene::SendTextureSettingsToShader()
{
	shaderPtr->Activate();
	shaderPtr->setFloat("textureScale", textureSettings.textureScale);
	shaderPtr->setFloat("normalStrength", textureSettings.normalStrength);
}

void TexturePlanetScene::UpdateSpecific(float deltaTime)
{
	UpdateWaterPosition(deltaTime);
	SendTextureSettingsToShader();
}

void TexturePlanetScene::RenderSpecific()
{
	waterTexture->Bind();
	noiseTexture->Bind();
	diffuseArray->Bind();
	normalArray->Bind();

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
	ImGui::Text("Terrain textures");
	ImGui::SliderFloat("Texture scale", &textureSettings.textureScale, 2.0f, 100.0f);
	ImGui::SliderFloat("Normal strength", &textureSettings.normalStrength, 0.0f, 2.0f);

	ImGui::Separator();
	ImGui::Text("Water Animation");
	ImGui::SliderFloat2("Water Speed", glm::value_ptr(textureSettings.waterSpeed), -0.1f, 0.1f);
}

void TexturePlanetScene::RenderGuiSpecific()
{
	if (ImGui::CollapsingHeader("Textures settings"))
		RenderTextureSettings();
}