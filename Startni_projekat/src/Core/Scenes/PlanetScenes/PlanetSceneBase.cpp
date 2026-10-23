#include "PlanetSceneBase.h"

PlanetSceneBase::PlanetSceneBase(ApplicationBase* app) : Scene(app)
{
	cameraPtr = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 250.0f));
	galaxyPtr = std::make_unique<Galaxy>();
	sunPtr = std::make_unique<Sun>();

	heightCalculatorPtr = std::make_unique<TerrainHeightCalculator>(&terrainSettings);

	surfaceCameraPtr = std::make_unique<SurfaceCamera>();
	surfaceCameraPtr->SetHeightCalculator(heightCalculatorPtr.get());
	surfaceCameraPtr->SetPlanetRadius(planetSettings.radius);

	RefreshMaxElevationCache();
}

PlanetSceneBase::~PlanetSceneBase() { }

void PlanetSceneBase::ApplyElevationRange(const ElevationRange& range)
{
	LOG_FUNC();
	cachedMaxElevation = std::max(range.maxElevation, 0.0f) * cameraMargin;
	surfaceCameraPtr->SetMaxElevationFallback(cachedMaxElevation);

	elevationRange.x = std::min(range.minElevation, -1e-3f);
	elevationRange.y = std::max(range.maxElevation * shadingMargin, 1e-3f);

	if (shaderPtr)
	{
		LOG_INFO("Evaluate range applyed max = {}, min = {}", elevationRange.x, elevationRange.y);
		shaderPtr->Activate();
		shaderPtr->setVec2("heightRange", elevationRange);
	}
}

void PlanetSceneBase::RefreshMaxElevationCache()
{
	ApplyElevationRange(heightCalculatorPtr->EstimateElevationRange(rangeSamples));
}

void PlanetSceneBase::PollElevationRange()
{
	if (rangeFuture.valid() &&
		rangeFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	{
		ApplyElevationRange(rangeFuture.get());
		OnMaxElevationChanged(cachedMaxElevation);
	}

	if (rangeDirty && !rangeFuture.valid())
	{
		rangeDirty = false;

		rangeFuture = std::async(std::launch::async, [snapshot = terrainSettings]()
			{
				TerrainHeightCalculator calc(&snapshot);
				return calc.EstimateElevationRange(rangeSamples);
			});
	}
}


void PlanetSceneBase::Start()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	SendNoiseSettingsToShader();

	StartSpecific();
}

void PlanetSceneBase::UpdateSunPosition(float deltaTime)
{
	const bool rotatePlanetNow = sunSettings.rotatePlanet && !IsWalking();

	if (rotatePlanetNow)
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
	sunSettings.direction = glm::normalize(sunPosition);
}

void PlanetSceneBase::Update(float deltaTime)
{
	GLFWwindow* win = app->GetGLFWWindow();
	WindowData* data = (WindowData*)glfwGetWindowUserPointer(win);

	PollElevationRange();
	surfaceCameraPtr->SetPlanetRotation(sunSettings.planetAngle);
	UpdateWalkToggle(win);

	PlanetInfluence influence;
	influence.planetWorldPosition = glm::vec3(0.0f);
	influence.radius = planetSettings.radius;
	influence.maxTerrainHeight = planetSettings.radius * cachedMaxElevation;
	cameraController.SetSingleInfluence(influence);

	const glm::vec3 activePos = IsWalking() ? surfaceCameraPtr->GetPosition() : cameraPtr->GetPosition();
	currentNearPlane = cameraController.ComputeNearPlane(activePos);
	if (!IsWalking())
		cameraPtr->SetSpeed(cameraController.ComputeSpeed(activePos));

	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
	{
		if (IsWalking())
			surfaceCameraPtr->Inputs(win, deltaTime, data->width, data->height);
		else
			cameraPtr->Inputs(win, deltaTime, data->width, data->height);
	}

	UpdateSunPosition(deltaTime);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, sunSettings.planetAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(planetSettings.radius));

	shaderPtr->Activate();

	shaderPtr->setMatrix("model", model);
	glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
	shaderPtr->setMat3("normalMatrix", normalMatrix);

	if (IsWalking())
		surfaceCameraPtr->SetPositionToShader("viewPos", *shaderPtr);
	else
		cameraPtr->SetPositionToShader("viewPos", *shaderPtr);


	planetSettings.matPtr->SetShaderProgramParameters(*shaderPtr, "material");
	sunSettings.lightPtr->SetShaderProgramParameters(*shaderPtr, "dirLight");

	shaderPtr->setInt("resolution", static_cast<int>(planetSettings.resolution));

	UpdateSpecific(deltaTime);
}

void PlanetSceneBase::Render()
{
	GLCall(glClearColor(0.5f, 0.5f, 0.5f, 1.0f));
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	GLFWwindow* win = app->GetGLFWWindow();
	WindowData* data = (WindowData*)glfwGetWindowUserPointer(win);

	const bool walking = IsWalking();

	glm::mat4 viewMat = walking ? surfaceCameraPtr->GetViewMatrix() : cameraPtr->GetViewMatrix();
	glm::mat4 projectionMat = walking
		? surfaceCameraPtr->GetProjectionMatrix(fov, currentNearPlane, cameraFarPlane, data->width, data->height)
		: cameraPtr->GetProjectionMatrix(fov, currentNearPlane, cameraFarPlane, data->width, data->height);

	galaxyPtr->Render(viewMat, projectionMat);
	sunPtr->Render(viewMat, projectionMat, sunSettings.direction);

	shaderPtr->Activate();
	if (walking)
		surfaceCameraPtr->Matrix(fov, currentNearPlane, cameraFarPlane, *shaderPtr, "camMat", data->width, data->height);
	else
		cameraPtr->Matrix(fov, currentNearPlane, cameraFarPlane, *shaderPtr, "camMat", data->width, data->height);

	RenderSpecific();
}

void PlanetSceneBase::RenderSunGui()
{
	ImGui::SliderFloat("Sun Velocity", &sunSettings.sunVelocity, 0.0f, 3.0f);
	ImGui::SliderFloat("Sun Height", &sunSettings.sunHeight, -10.0f, 10.0f);
	ImGui::SliderFloat("Sun Distance", &sunSettings.sunDistance, 1.0f, 50.0f);

	ImGui::BeginDisabled(IsWalking());
	ImGui::Checkbox("Rotate planet (freezes sun)", &sunSettings.rotatePlanet);
	ImGui::SliderFloat("Planet Velocity", &sunSettings.planetVelocity, 0.0f, 3.0f);
	ImGui::EndDisabled();

	if (IsWalking()) 
		ImGui::TextDisabled("Planet rotation disabled while walking");

	if (ImGui::Button("Reset Angles"))
	{
		sunSettings.sunAngle = 0.0f;
		sunSettings.planetAngle = 0.0f;
	}

	ImGui::Separator();
	SunVisualSettings& sv = sunPtr->GetSettings();
	ImGui::Checkbox("Show sun", &sv.visible);
	ImGui::SliderFloat("Sun radius (deg)", &sv.angularRadiusDeg, 0.2f, 10.0f);
	ImGui::SliderFloat("Glow size", &sv.glowScale, 3.0f, 20.0f);
	ImGui::SliderFloat("Glow strength", &sv.glowStrength, 0.0f, 2.0f);
	ImGui::SliderFloat("Core intensity", &sv.coreIntensity, 0.5f, 6.0f);
	ImGui::ColorEdit3("Sun color", glm::value_ptr(sv.color));
}

void PlanetSceneBase::RenderCameraGui()
{
	ImGui::Text("Mode: %s  (F to toggle)", IsWalking() ? "Walking" : "Free flight");
	if (ImGui::Button(IsWalking() ? "Switch to free flight" : "Switch to walking"))
	{
		if (!IsWalking())
			surfaceCameraPtr->EnterFromWorld(cameraPtr->GetPosition(), cameraPtr->GetOrientation());
		else
		{
			cameraPtr->SetPosition(surfaceCameraPtr->GetExitPosition());
			cameraPtr->SetOrientation(surfaceCameraPtr->GetExitOrientation());
			surfaceCameraPtr->SetActive(false);
		}
	}

	ImGui::Separator();
	CameraAltitudeSettings& s = cameraController.GetSettingsRef();

	ImGui::Text("Altitude: %.2f", cameraController.GetAltitude(cameraPtr->GetPosition()));
	ImGui::Text("Current speed: %.2f u/s", cameraPtr->GetSpeed());
	ImGui::Text("Current near plane: %.3f", currentNearPlane);

	ImGui::Separator();
	ImGui::SliderFloat("Speed per unit altitude", &s.speedPerUnitAltitude, 0.1f, 20.0f);
	ImGui::SliderFloat("Min speed", &s.minSpeed, 0.1f, 50.0f);
	ImGui::SliderFloat("Max speed", &s.maxSpeed, 10.0f, 5000.0f);

	ImGui::Separator();
	ImGui::SliderFloat("Near per unit altitude", &s.nearPerUnitAltitude, 0.001f, 0.2f);
	ImGui::SliderFloat("Min near", &s.minNear, 0.01f, 1.0f);
	ImGui::SliderFloat("Max near", &s.maxNear, 0.5f, 50.0f);
}


void PlanetSceneBase::SendNoiseSettingsToShader()
{
	shaderPtr->Activate();

	const int count = std::min(static_cast<int>(terrainSettings.layers.size()), TerrainShapeSettings::maxLayers);
	shaderPtr->setInt("layerCount", count);

	for (int i = 0; i < count; i++)
	{
		const TerrainLayerSettings& layer = terrainSettings.layers[i];
		const NoiseSettings& n = layer.noise;
		const std::string p = "layers[" + std::to_string(i) + "].";

		shaderPtr->setFloat(p + "n.strength", n.strength);
		shaderPtr->setInt(p + "n.numberOfOctaves", n.numberOfOctaves);
		shaderPtr->setFloat(p + "n.baseRoughness", n.baseRoughness);
		shaderPtr->setFloat(p + "n.roughness", n.roughness);
		shaderPtr->setFloat(p + "n.persistance", n.persistance);
		shaderPtr->setFloat(p + "n.minValue", n.minValue);
		shaderPtr->setVec3(p + "n.center", n.center);
		shaderPtr->setUint(p + "n.seed", n.seed);

		shaderPtr->setInt(p + "filterType", static_cast<int>(layer.filterType));
		shaderPtr->setInt(p + "enabled", layer.enabled ? 1 : 0);
		shaderPtr->setInt(p + "useFirstLayerAsMask", layer.useFirstLayerAsMask ? 1 : 0);
		shaderPtr->setFloat(p + "weightMultiplier", layer.weightMultiplier);
	}

	shaderPtr->setVec2("heightRange", elevationRange);
}

void PlanetSceneBase::RenderNoiseSettings()
{
	bool changed = false;

	for (size_t i = 0; i < terrainSettings.layers.size(); ++i)
	{
		TerrainLayerSettings& layer = terrainSettings.layers[i];
		NoiseSettings& n = layer.noise;

		ImGui::PushID(static_cast<int>(i));

		const std::string header = "Noise layer " + std::to_string(i + 1);
		if (ImGui::CollapsingHeader(header.c_str()))
		{
			if (ImGui::Button("Randomize Seed"))
			{
				std::random_device rd;
				n.seed = static_cast<unsigned int>(rd());
				changed = true;
			}

			const char* filterNames[] = { "Simple", "Rigid" };
			int type = static_cast<int>(layer.filterType);
			if (ImGui::Combo("Filter type", &type, filterNames, IM_ARRAYSIZE(filterNames)))
			{
				layer.filterType = static_cast<NoiseFilterType>(type);
				changed = true;
			}

			if (i > 0)
			{
				changed |= ImGui::Checkbox("Enable layer", &layer.enabled);
				changed |= ImGui::Checkbox("Use first layer as mask", &layer.useFirstLayerAsMask);
			}

			changed |= ImGui::SliderInt("Number of octavs", &n.numberOfOctaves, 1, 8);
			changed |= ImGui::SliderFloat("Strength", &n.strength, 0.0f, 2.0f);
			changed |= ImGui::SliderFloat("Base roughness", &n.baseRoughness, 0.1f, 4.0f);
			changed |= ImGui::SliderFloat("Roughness", &n.roughness, 1.0f, 4.0f);
			changed |= ImGui::SliderFloat("Persistance", &n.persistance, 0.0f, 1.0f);
			changed |= ImGui::SliderFloat("Min value", &n.minValue, 0.0f, 2.0f);
			changed |= ImGui::DragFloat3("Center", glm::value_ptr(n.center), 0.01f);

			if (layer.filterType == NoiseFilterType::Rigid)
				changed |= ImGui::SliderFloat("Weight multiplier", &layer.weightMultiplier, 0.0f, 2.0f);
		}

		ImGui::PopID();
	}

	if (terrainSettings.CanAddLayer() && ImGui::Button("Add layer"))
	{
		std::random_device rd;
		terrainSettings.AddRigidLayer(static_cast<unsigned int>(rd()));
		changed = true;
	}
	if (terrainSettings.layers.size() > 1)
	{
		ImGui::SameLine();
		if (ImGui::Button("Remove layer"))
		{
			terrainSettings.layers.pop_back();
			changed = true;
		}
	}

	if (changed)
	{
		SendNoiseSettingsToShader();
		RequestElevationRefresh();
	}
}

void PlanetSceneBase::OnImGuiRender()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	if (ImGui::CollapsingHeader("Planet Settings"))
		RenderPlanetPropertiesGui();

	if (ImGui::CollapsingHeader("Camera"))
		RenderCameraGui();

	if (ImGui::CollapsingHeader("Sun & Lighting"))
		RenderSunGui();

	if (ImGui::CollapsingHeader("Noise settings"))
		RenderNoiseSettings();

	RenderGuiSpecific();

	ImGui::Text("Elevation range: %.3f .. %.3f", elevationRange.x, elevationRange.y);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
}

void PlanetSceneBase::UpdateWalkToggle(GLFWwindow* window)
{
	const bool pressed = (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS);

	if (pressed && !walkKeyWasPressed)
	{
		if (!IsWalking())
		{
			surfaceCameraPtr->EnterFromWorld(cameraPtr->GetPosition(), cameraPtr->GetOrientation());
		}
		else
		{
			cameraPtr->SetPosition(surfaceCameraPtr->GetExitPosition());
			cameraPtr->SetOrientation(surfaceCameraPtr->GetExitOrientation());
			surfaceCameraPtr->SetActive(false);
		}
	}

	walkKeyWasPressed = pressed;
}


void PlanetSceneBase::OnScroll(double xoffset, double yoffset)
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