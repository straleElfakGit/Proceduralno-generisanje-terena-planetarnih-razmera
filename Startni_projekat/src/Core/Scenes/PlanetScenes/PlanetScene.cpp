#include "PlanetScene.h"

PlanetScene::PlanetScene(ApplicationBase* app) : PlanetSceneBase(app)
{
	ResourceManager& rm = ResourceManager::GetInstance();

	shaderPtr = rm.GetOrLoadShader("PlanetShader", "assets/Shaders/simplePlanetShader.vert", "assets/Shaders/simplePlanetShader.frag");

	decoyCameraPtr = std::make_unique<DecoyCamera>();
}

int PlanetScene::ComputeMaxLevelForRadius(float radius) const
{
	const float approxRootSize = 2.0f * std::max(radius, 0.001f);
	const float ratio = std::max(approxRootSize / targetLeafSize, 1.0f);
	const int computed = static_cast<int>(std::ceil(std::log2(ratio)));

	return glm::clamp(computed, 2, maxPlanetLods - 1);
}

void PlanetScene::SendBiomeNoiseSettingsToShader()
{
	shaderPtr->Activate();

	ColorSettings& cSettings = colorGenerator.GetSettings();
	NoiseSettings& n = cSettings.biomeSettings.noise;

	shaderPtr->setFloat("biomeNoise.strength", n.strength);
	shaderPtr->setInt("biomeNoise.numberOfOctaves", n.numberOfOctaves);
	shaderPtr->setFloat("biomeNoise.baseRoughness", n.baseRoughness);
	shaderPtr->setFloat("biomeNoise.roughness", n.roughness);
	shaderPtr->setFloat("biomeNoise.persistance", n.persistance);
	shaderPtr->setFloat("biomeNoise.minValue", n.minValue);
	shaderPtr->setVec3("biomeNoise.center", n.center);
	shaderPtr->setUint("biomeNoise.seed", n.seed);
}

void PlanetScene::StartSpecific()
{
	PlanetQuadTreeSettings qts;
	qts.planetRadius = planetSettings.radius;
	qts.maxLevel = ComputeMaxLevelForRadius(planetSettings.radius);
	qts.nodeResolution = static_cast<int>(planetSettings.resolution);
	qts.lodRangeFactor = 6.0f;
	qts.morphStartRatio = 0.5f;
	qts.heightMax = cachedMaxElevation;
	qts.frustumCulling = true;
	qts.horizonCulling = true;
	planetQuadTree.Configure(qts);

	InitGridMesh();

	nodeSSBOPtr = std::make_unique<SSBO<PlanetNode>>(c_NodeSSBOBinding);

	decoyCameraPtr->SetRadius(planetSettings.radius * 1.25f);
	SendBiomeNoiseSettingsToShader();
	colorGenerator.UpdateElevation(elevationRange);
}

void PlanetScene::InitGridMesh()
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

void PlanetScene::UpdateSpecific(float deltaTime)
{
	GLFWwindow* win = app->GetGLFWWindow();
	WindowData* data = (WindowData*)glfwGetWindowUserPointer(win);

	decoyCameraPtr->Update(deltaTime);

	shaderPtr->Activate();
	shaderPtr->setInt("maxLevel", planetQuadTree.GetSettings().maxLevel);
	shaderPtr->setFloatArray("morphStart", planetQuadTree.GetMorphStart(), maxPlanetLods);
	shaderPtr->setFloatArray("morphEnd", planetQuadTree.GetMorphEnd(), maxPlanetLods);


	glm::mat4 planetTransform = glm::mat4(1.0f);
	planetTransform = glm::rotate(planetTransform, sunSettings.planetAngle, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::vec3 activeCamWorldPos;
	glm::mat4 activeViewMat;
	glm::mat4 activeProjMat;

	if (decoyCameraPtr->IsActive())
	{
		activeCamWorldPos = decoyCameraPtr->GetPosition();
		activeViewMat = decoyCameraPtr->GetViewMatrix();
		activeProjMat = decoyCameraPtr->GetProjectionMatrix(data->width, data->height);
	}
	else if (IsWalking())
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
	nodeSSBOPtr->Upload(planetQuadTree.GetSelection());

	colorGenerator.SetShaderProgramParameters(*shaderPtr);
}

void PlanetScene::RenderSpecific()
{
	GLFWwindow* win = app->GetGLFWWindow();
	WindowData* data = (WindowData*)glfwGetWindowUserPointer(win);

	glm::mat4 viewMat = IsWalking()
		? surfaceCameraPtr->GetViewMatrix()
		: cameraPtr->GetViewMatrix();
	glm::mat4 projectionMat = IsWalking()
		? surfaceCameraPtr->GetProjectionMatrix(fov, currentNearPlane, cameraFarPlane, data->width, data->height)
		: cameraPtr->GetProjectionMatrix(fov, currentNearPlane, cameraFarPlane, data->width, data->height);

	if (planetSettings.showMesh)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	nodeSSBOPtr->Bind();

	gridVAOPtr->Bind();
	const GLsizei instanceCount = static_cast<GLsizei>(planetQuadTree.GetSelection().size());
	if (instanceCount > 0)
		glDrawElementsInstanced(GL_TRIANGLES, gridIndexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
	gridVAOPtr->Unbind();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (decoyCameraPtr->IsActive())
		decoyCameraPtr->Render(projectionMat, viewMat);
}

void PlanetScene::RenderPlanetPropertiesGui()
{
	ImGui::Checkbox("Show Mesh", &planetSettings.showMesh);

	int currentRes = static_cast<int>(planetSettings.resolution);
	if (ImGui::SliderInt("Node resolution", &currentRes, 2, 128))
	{
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
		decoyCameraPtr->SetRadius(currentRadius * 1.25f);
	}
}

void PlanetScene::SetMaxElevation(float maxElevation)
{
	/*PlanetQuadTreeSettings qts = planetQuadTree.GetSettings();
	qts.heightMax = maxElevation;
	planetQuadTree.Configure(qts);*/
}

void PlanetScene::OnMaxElevationChanged(float maxElevation)
{
	planetQuadTree.SetHeightMax(elevationRange.y);
	colorGenerator.UpdateElevation(elevationRange);
}

void PlanetScene::RenderGuiSpecific()
{
	if (ImGui::CollapsingHeader("Culling & Decoy Camera Debug"))
		decoyCameraPtr->RenderGui();
	if (ImGui::CollapsingHeader("Bioms and coloring"))
		RenderColorGui();
}

void PlanetScene::RenderColorGui()
{
	BiomeColorSettings& biomeSettings = colorGenerator.GetSettings().biomeSettings;

	bool colorChanged = false;
	bool structuralChanged = false;

	structuralChanged |= ImGui::SliderFloat("Noise Offset", &biomeSettings.noiseOffset, -1.0f, 1.0f);
	structuralChanged |= ImGui::SliderFloat("Noise Strength", &biomeSettings.noiseStrength, 0.0f, 1.0f);
	structuralChanged |= ImGui::SliderFloat("Blend Amount", &biomeSettings.blendAmount, 0.0f, 1.0f);

	ImGui::Separator();

	for (size_t i = 0; i < biomeSettings.biomes.size(); i++)
	{
		Biome& biome = biomeSettings.biomes[i];
		ImGui::PushID(static_cast<int>(i));

		std::string header = "Biome " + std::to_string(i + 1);
		if (ImGui::CollapsingHeader(header.c_str()))
		{
			structuralChanged |= ImGui::SliderFloat("Start Height", &biome.startHeight, 0.0f, 1.0f);
			colorChanged |= ImGui::ColorEdit3("Tint", glm::value_ptr(biome.tint));
			colorChanged |= ImGui::SliderFloat("Tint Amount", &biome.tintPercent, 0.0f, 1.0f);

			for (size_t k = 0; k < biome.gradient.keys.size(); k++)
			{
				ImGui::PushID(static_cast<int>(k));
				colorChanged |= ImGui::ColorEdit3("##color", glm::value_ptr(biome.gradient.keys[k].color), ImGuiColorEditFlags_NoInputs);
				ImGui::SameLine();
				colorChanged |= ImGui::SliderFloat("##pos", &biome.gradient.keys[k].position, 0.0f, 1.0f);
				ImGui::PopID();
			}
			if (ImGui::Button("Add key")) { biome.gradient.keys.push_back({ 1.0f, glm::vec3(1.0f) }); colorChanged = true; }
			ImGui::SameLine();
			if (ImGui::Button("Remove key") && biome.gradient.keys.size() > 2) { biome.gradient.keys.pop_back(); colorChanged = true; }
		}
		ImGui::PopID();
	}

	if (ImGui::Button("Add biome") && biomeSettings.biomes.size() < 8)
	{
		Biome newBiome;
		newBiome.gradient.keys = { { 0.0f, glm::vec3(0.5f) }, { 1.0f, glm::vec3(1.0f) } };
		newBiome.startHeight = 1.0f;
		biomeSettings.biomes.push_back(newBiome);
		structuralChanged = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Remove biome") && biomeSettings.biomes.size() > 1)
	{
		biomeSettings.biomes.pop_back();
		structuralChanged = true;
	}

	if (colorChanged || structuralChanged)
		colorGenerator.UpdateColors();
}