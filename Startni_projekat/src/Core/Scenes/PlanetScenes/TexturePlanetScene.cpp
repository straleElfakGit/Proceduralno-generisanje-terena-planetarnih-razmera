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

	diffuseArray->SetAnisotropy(8.0f); 
	normalArray->SetAnisotropy(4.0f);
}

TexturePlanetScene::~TexturePlanetScene() { }

int TexturePlanetScene::ComputeMaxLevelForRadius(float radius) const
{
	const float approxRootSize = 2.0f * std::max(radius, 0.001f);
	const float ratio = std::max(approxRootSize / targetLeafSize, 1.0f);
	const int computed = static_cast<int>(std::ceil(std::log2(ratio)));

	return glm::clamp(computed, 2, maxPlanetLods - 1);
}

void TexturePlanetScene::StartSpecific()
{
	int res = glm::clamp(static_cast<int>(planetSettings.resolution), 2, kMaxNodeResolution);
	res &= ~1;
	planetSettings.resolution = static_cast<unsigned int>(res);

	PlanetQuadTreeSettings qts;
	qts.planetRadius = planetSettings.radius;
	qts.maxLevel = ComputeMaxLevelForRadius(planetSettings.radius);
	qts.nodeResolution = res;
	qts.lodRangeFactor = 5.0f;
	qts.morphStartRatio = 0.5f;
	qts.heightMax = cachedMaxElevation;
	qts.frustumCulling = true;
	qts.horizonCulling = true;
	planetQuadTree.Configure(qts);

	InitGridMesh();

	nodeSSBOPtr = std::make_unique<SSBO<PlanetNode>>(c_NodeSSBOBinding);
}

void TexturePlanetScene::InitGridMesh()
{
	const int res = std::max(1, static_cast<int>(planetSettings.resolution));

	std::vector<unsigned int> indices;
	indices.reserve(static_cast<size_t>(res) * res * 6);

	auto idx = [res](int gx, int gy) -> unsigned int {
		return static_cast<unsigned int>(gy * (res + 1) + gx);
		};

	for (int j = 0; j < res; ++j)
	{
		for (int i = 0; i < res; ++i)
		{
			indices.push_back(idx(i, j));
			indices.push_back(idx(i, j + 1));
			indices.push_back(idx(i + 1, j + 1));

			indices.push_back(idx(i, j));
			indices.push_back(idx(i + 1, j + 1));
			indices.push_back(idx(i + 1, j));
		}
	}
	gridIndexCount = static_cast<GLsizei>(indices.size());

	gridVAOPtr = std::make_unique<VAO<GLuint>>();
	gridVAOPtr->Bind();

	gridEBOPtr = std::make_unique<EBO<GLuint>>(indices);

	gridVAOPtr->Unbind();
}

void TexturePlanetScene::OnMaxElevationChanged(float maxElevation)
{
	planetQuadTree.SetHeightMax(elevationRange.y);
}

void TexturePlanetScene::UpdateLod(float deltaTime)
{
	GLFWwindow* win = app->GetGLFWWindow();
	WindowData* data = (WindowData*)glfwGetWindowUserPointer(win);

	shaderPtr->setInt("maxLevel", planetQuadTree.GetSettings().maxLevel);
	shaderPtr->setFloatArray("morphStart", planetQuadTree.GetMorphStart(), maxPlanetLods);
	shaderPtr->setFloatArray("morphEnd", planetQuadTree.GetMorphEnd(), maxPlanetLods);

	glm::mat4 planetTransform = glm::mat4(1.0f);
	planetTransform = glm::rotate(planetTransform, sunSettings.planetAngle, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::vec3 activeCamWorldPos;
	glm::mat4 activeViewMat;
	glm::mat4 activeProjMat;

	if (IsWalking())
	{
		activeCamWorldPos = surfaceCameraPtr->GetPosition();
		activeViewMat = surfaceCameraPtr->GetViewMatrix();
		activeProjMat = surfaceCameraPtr->GetProjectionMatrix(fov, currentNearPlane, cameraFarPlane, data->width, data->height);
	}
	else
	{
		activeCamWorldPos = cameraPtr->GetPosition();
		activeViewMat = cameraPtr->GetViewMatrix();
		activeProjMat = cameraPtr->GetProjectionMatrix(fov, currentNearPlane, cameraFarPlane, data->width, data->height);
	}

	const glm::vec3 cameraLocal = glm::vec3(glm::inverse(planetTransform) * glm::vec4(activeCamWorldPos, 1.0f));

	PlanetView view;
	view.cameraInLocalPlanetSpace = cameraLocal;
	view.localToClip = activeProjMat * activeViewMat * planetTransform;

	planetQuadTree.Select(view);
	auto selection = planetQuadTree.GetSelection();
	std::sort(selection.begin(), selection.end(),
		[](const PlanetNode& a, const PlanetNode& b) { return a.lod < b.lod; });
	nodeSSBOPtr->Upload(selection);
}


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
	UpdateLod(deltaTime);
}

void TexturePlanetScene::RenderSpecific()
{
	GLFWwindow* win = app->GetGLFWWindow();
	WindowData* data = (WindowData*)glfwGetWindowUserPointer(win);

	waterTexture->Bind();
	noiseTexture->Bind();
	diffuseArray->Bind();
	normalArray->Bind();

	if (planetSettings.showMesh)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	nodeSSBOPtr->Bind();

	gridVAOPtr->Bind();
	const GLsizei instanceCount = static_cast<GLsizei>(planetQuadTree.GetSelection().size());
	if (instanceCount > 0)
		glDrawElementsInstanced(GL_TRIANGLES, gridIndexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
	gridVAOPtr->Unbind();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void TexturePlanetScene::RenderPlanetPropertiesGui()
{
	ImGui::Checkbox("Show Mesh", &planetSettings.showMesh);

	int currentRes = static_cast<int>(planetSettings.resolution);
	if (ImGui::SliderInt("Node resolution", &currentRes, 2, 256)) {
		currentRes = std::max(currentRes & ~1, 2);
		planetSettings.resolution = static_cast<unsigned int>(currentRes);

		PlanetQuadTreeSettings qts = planetQuadTree.GetSettings();
		qts.nodeResolution = currentRes;
		planetQuadTree.Configure(qts);
		InitGridMesh();

	}

	float currentRadius = planetSettings.radius;
	if (ImGui::SliderFloat("Radius", &currentRadius, 5.0f, 200.0f))
	{
		planetSettings.radius = currentRadius;
		LOG_INFO("Radius set to {}", planetSettings.radius);

		PlanetQuadTreeSettings qts = planetQuadTree.GetSettings();
		qts.planetRadius = currentRadius;
		qts.maxLevel = ComputeMaxLevelForRadius(currentRadius);
		planetQuadTree.Configure(qts);

		surfaceCameraPtr->SetPlanetRadius(currentRadius);
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