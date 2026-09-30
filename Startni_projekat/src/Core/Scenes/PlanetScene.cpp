#include "PlanetScene.h"

PlanetScene::PlanetScene(ApplicationBase* app) : Scene(app)
{
	ResourceManager& rm = ResourceManager::GetInstance();

	shaderPtr = rm.GetOrLoadShader("PlanetShader", "assets/Shaders/simplePlanetShader.vert", "assets/Shaders/simplePlanetShader.frag");
	cameraPtr = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));

	galaxyShaderPtr = rm.GetOrLoadShader("SkyboxShader", "assets/Shaders/skyBoxShader.vert", "assets/Shaders/skyBoxShader.frag");
	galaxyPtr = std::make_unique<Galaxy>(*galaxyShaderPtr);

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
}

PlanetScene::~PlanetScene()
{
	if (gridVAO != 0)
	{
		glDeleteVertexArrays(1, &gridVAO);
		gridVAO = 0;
	}

	if (gridEBO != 0)
	{
		glDeleteBuffers(1, &gridEBO);
		gridEBO = 0;
	}

	if (nodeSSBO != 0)
	{
		glDeleteBuffers(1, &nodeSSBO);
		nodeSSBO = 0;
	}
}

void PlanetScene::Start()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	SendNoiseSettingsToShader();

	PlanetQuadTreeSettings qts;
	qts.planetRadius = planetSettings.radius;
	qts.maxLevel = 8;                                     
	qts.nodeResolution = static_cast<int>(planetSettings.resolution);
	qts.lodRangeFactor = 6.0f;
	qts.morphStartRatio = 0.5f;
	qts.heightMax = 0.0f;                                 
	qts.frustumCulling = true;
	qts.horizonCulling = true;
	planetQuadTree.Configure(qts);

	InitGridMesh();
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

	if (gridVAO == 0)
		glGenVertexArrays(1, &gridVAO);
	glBindVertexArray(gridVAO);

	if (gridEBO == 0)
		glGenBuffers(1, &gridEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
		indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void PlanetScene::UploadSelection(const std::vector<PlanetNode>& sel)
{
	if (nodeSSBO == 0)
		glGenBuffers(1, &nodeSSBO);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, nodeSSBO);

	glBufferData(GL_SHADER_STORAGE_BUFFER,
		static_cast<GLsizeiptr>(sel.size() * sizeof(PlanetNode)),
		sel.empty() ? nullptr : sel.data(),
		GL_DYNAMIC_DRAW);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, nodeSSBO);
}


void PlanetScene::UpdateSunPosition(float deltaTime)
{
	if (sunSettings.rotatePlanet)
		sunSettings.planetAngle += sunSettings.planetVelocity * deltaTime;
	else
		sunSettings.sunAngle += sunSettings.sunVelocity * deltaTime;

	sunSettings.planetAngle = std::fmod(sunSettings.planetAngle, glm::two_pi<float>());
	sunSettings.sunAngle = std::fmod(sunSettings.sunAngle, glm::two_pi<float>());

	glm::vec3 sunPosition = glm::vec3(
		std::cos(sunSettings.sunAngle) * sunSettings.sunDistance,
		sunSettings.sunHeight,
		std::sin(sunSettings.sunAngle) * sunSettings.sunDistance);

	sunSettings.lightPtr->UpdateDirection(glm::normalize(-sunPosition));
}

void PlanetScene::UpdateWaterPosition(float deltaTime)
{
	textureSettings.waterOffset += textureSettings.waterSpeed * deltaTime;
	textureSettings.waterOffset.x = std::fmod(textureSettings.waterOffset.x, 1.0f);
	textureSettings.waterOffset.y = std::fmod(textureSettings.waterOffset.y, 1.0f);

	shaderPtr->Activate();

	shaderPtr->setVec2("waterOffset", textureSettings.waterOffset);
}

void PlanetScene::Update(float deltaTime)
{
	GLFWwindow* win = app->GetGLFWWindow();
	WindowData* data = (WindowData*)glfwGetWindowUserPointer(win);

	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
		cameraPtr->Inputs(win, deltaTime, data->width, data->height);

	UpdateSunPosition(deltaTime);
	UpdateWaterPosition(deltaTime);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, sunSettings.planetAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(planetSettings.radius));

	shaderPtr->Activate();

	shaderPtr->setMatrix("model", model);
	glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
	shaderPtr->setMat3("normalMatrix", normalMatrix);

	cameraPtr->SetPositionToShader("viewPos", *shaderPtr);

	planetSettings.matPtr->SetShaderProgramParameters(*shaderPtr, "material");
	sunSettings.lightPtr->SetShaderProgramParameters(*shaderPtr, "dirLight");

	shaderPtr->setInt("resolution", static_cast<int>(planetSettings.resolution));
	shaderPtr->setInt("maxLevel", planetQuadTree.GetSettings().maxLevel);


	glm::mat4 planetTransform = glm::mat4(1.0f);
	planetTransform = glm::rotate(planetTransform, sunSettings.planetAngle, glm::vec3(0.0f, 1.0f, 0.0f));

	const glm::vec3 cameraWorldPos = cameraPtr->GetPosition();
	const glm::vec3 cameraLocal = glm::vec3(glm::inverse(planetTransform) * glm::vec4(cameraWorldPos, 1.0f));

	const glm::mat4 viewMat = cameraPtr->GetViewMatrix();
	const glm::mat4 projectionMat = cameraPtr->GetProjectionMatrix(fov, 0.1f, 1000.0f, data->width, data->height);

	PlanetView view;
	view.cameraInLocalPlanetSpace = cameraLocal;
	view.localToClip = projectionMat * viewMat * planetTransform;

	planetQuadTree.Select(view);
	UploadSelection(planetQuadTree.GetSelection());
}

void PlanetScene::Render()
{
	GLCall(glClearColor(0.5f, 0.5f, 0.5f, 1.0f));
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	GLFWwindow* win = app->GetGLFWWindow();
	WindowData* data = (WindowData*)glfwGetWindowUserPointer(win);

	glm::mat4 viewMat = cameraPtr->GetViewMatrix();
	glm::mat4 projectionMat = cameraPtr->GetProjectionMatrix(fov, 0.1f, 1000.0f, data->width, data->height);
	galaxyPtr->Render(viewMat, projectionMat);

	shaderPtr->Activate();
	cameraPtr->Matrix(fov, 0.1f, 1000.0f, *shaderPtr, "camMat", data->width, data->height);

	for (int i = 0; i < 7; i++)
		textures[i]->Bind();

	if (planetSettings.showMesh)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindVertexArray(gridVAO);
	const GLsizei instanceCount = static_cast<GLsizei>(planetQuadTree.GetSelection().size());
	if (instanceCount > 0)
		glDrawElementsInstanced(GL_TRIANGLES, gridIndexCount, GL_UNSIGNED_INT, nullptr, instanceCount);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
		planetQuadTree.Configure(qts);
	}
}

void PlanetScene::RenderSunGui()
{
	ImGui::SliderFloat("Sun Velocity", &sunSettings.sunVelocity, 0.0f, 3.0f);
	ImGui::SliderFloat("Sun Height", &sunSettings.sunHeight, -10.0f, 10.0f);
	ImGui::SliderFloat("Sun Distance", &sunSettings.sunDistance, 1.0f, 50.0f);

	ImGui::Checkbox("Rotate planet (freezes sun)", &sunSettings.rotatePlanet);
	ImGui::SliderFloat("Planet Velocity", &sunSettings.planetVelocity, 0.0f, 3.0f);

	if (ImGui::Button("Reset Angles"))
	{
		sunSettings.sunAngle = 0.0f;
		sunSettings.planetAngle = 0.0f;
	}
}

void PlanetScene::SendNoiseSettingsToShader()
{
	shaderPtr->Activate();

	shaderPtr->setFloat("noise.strength", noiseSettings.strength);
	shaderPtr->setInt("noise.numberOfOctaves", noiseSettings.numberOfOctaves);
	shaderPtr->setFloat("noise.baseRoughness", noiseSettings.baseRoughness);
	shaderPtr->setFloat("noise.roughness", noiseSettings.roughness);
	shaderPtr->setFloat("noise.persistance", noiseSettings.persistance);
	shaderPtr->setFloat("noise.minValue", noiseSettings.minValue);
	shaderPtr->setVec3("noise.center", noiseSettings.center);
	shaderPtr->setUint("noise.seed", noiseSettings.seed);

	float maxElevation = noiseSettings.CalculateTheoreticalMaxElevation();
	shaderPtr->setFloat("maxElevation", maxElevation);
}

void PlanetScene::RenderNoiseSettings()
{
	if (ImGui::SliderInt("Number of octavs", &noiseSettings.numberOfOctaves, 1, 8))
	{
		shaderPtr->Activate();
		shaderPtr->setInt("noise.numberOfOctaves", noiseSettings.numberOfOctaves);
		float maxElevation = noiseSettings.CalculateTheoreticalMaxElevation();
		shaderPtr->setFloat("maxElevation", maxElevation);
	}

	if (ImGui::SliderFloat("Strength", &noiseSettings.strength, 0.0f, 2.0f))
	{
		shaderPtr->Activate();
		shaderPtr->setFloat("noise.strength", noiseSettings.strength);
		float maxElevation = noiseSettings.CalculateTheoreticalMaxElevation();
		shaderPtr->setFloat("maxElevation", maxElevation);
	}

	if (ImGui::SliderFloat("Base roughness", &noiseSettings.baseRoughness, 0.1f, 4.0f))
	{
		shaderPtr->Activate();
		shaderPtr->setFloat("noise.baseRoughness", noiseSettings.baseRoughness);
	}

	if (ImGui::SliderFloat("Roughness", &noiseSettings.roughness, 1.0f, 4.0f))
	{
		shaderPtr->Activate();
		shaderPtr->setFloat("noise.roughness", noiseSettings.roughness);
	}

	if (ImGui::SliderFloat("Persistance", &noiseSettings.persistance, 0.0f, 1.0f))
	{
		shaderPtr->Activate();
		shaderPtr->setFloat("noise.persistance", noiseSettings.persistance);
		float maxElevation = noiseSettings.CalculateTheoreticalMaxElevation();
		shaderPtr->setFloat("maxElevation", maxElevation);
	}

	if (ImGui::SliderFloat("Min value", &noiseSettings.minValue, 0.0f, 2.0f))
	{
		shaderPtr->Activate();
		shaderPtr->setFloat("noise.minValue", noiseSettings.minValue);
		float maxElevation = noiseSettings.CalculateTheoreticalMaxElevation();
		shaderPtr->setFloat("maxElevation", maxElevation);
	}

	if (ImGui::DragFloat3("Center", glm::value_ptr(noiseSettings.center), 0.01f))
	{
		shaderPtr->Activate();
		shaderPtr->setVec3("noise.center", noiseSettings.center);
	}
	if (ImGui::Button("Randomize Seed"))
	{
		std::random_device rd;
		noiseSettings.seed = static_cast<unsigned int>(rd());
		shaderPtr->Activate();
		shaderPtr->setUint("noise.seed", noiseSettings.seed);
	}
}

void PlanetScene::RenderTextureSettings()
{
	ImGui::Text("Water Animation");
	ImGui::SliderFloat2("Water Speed", glm::value_ptr(textureSettings.waterSpeed), -0.1f, 0.1f);
}

void PlanetScene::OnImGuiRender()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	if (ImGui::CollapsingHeader("Planet Settings"))
		RenderPlanetPropertiesGui();

	if (ImGui::CollapsingHeader("Sun & Lighting"))
		RenderSunGui();

	if (ImGui::CollapsingHeader("Textures settings"))
		RenderTextureSettings();

	if (ImGui::CollapsingHeader("Noise settings"))
		RenderNoiseSettings();

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
}

void PlanetScene::OnScroll(double xoffset, double yoffset)
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse)
		return;

	fov -= (float)yoffset;
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 45.0f)
		fov = 45.0f;
}