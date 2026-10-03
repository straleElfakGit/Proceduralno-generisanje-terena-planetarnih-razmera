#include "PlanetSceneBase.h"

PlanetSceneBase::PlanetSceneBase(ApplicationBase* app) : Scene(app)
{
	cameraPtr = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 23.0f));
	galaxyPtr = std::make_unique<Galaxy>();

	heightCalculatorPtr = std::make_unique<TerrainHeightCalculator>(&noiseSettings);

	surfaceCameraPtr = std::make_unique<SurfaceCamera>();
	surfaceCameraPtr->SetHeightCalculator(heightCalculatorPtr.get());
	surfaceCameraPtr->SetPlanetRadius(planetSettings.radius);
	surfaceCameraPtr->SetMaxElevationFallback(noiseSettings.CalculateTheoreticalMaxElevation());

}

PlanetSceneBase::~PlanetSceneBase() { }

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

void PlanetSceneBase::Update(float deltaTime)
{
	GLFWwindow* win = app->GetGLFWWindow();
	WindowData* data = (WindowData*)glfwGetWindowUserPointer(win);

	UpdateWalkToggle(win);

	PlanetInfluence influence;
	influence.planetWorldPosition = glm::vec3(0.0f);
	influence.radius = planetSettings.radius;
	influence.maxTerrainHeight = planetSettings.radius * noiseSettings.CalculateTheoreticalMaxElevation();
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

	ImGui::Checkbox("Rotate planet (freezes sun)", &sunSettings.rotatePlanet);
	ImGui::SliderFloat("Planet Velocity", &sunSettings.planetVelocity, 0.0f, 3.0f);

	if (ImGui::Button("Reset Angles"))
	{
		sunSettings.sunAngle = 0.0f;
		sunSettings.planetAngle = 0.0f;
	}
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

void PlanetSceneBase::RenderNoiseSettings()
{
	if (ImGui::SliderInt("Number of octavs", &noiseSettings.numberOfOctaves, 1, 8))
	{
		shaderPtr->Activate();
		shaderPtr->setInt("noise.numberOfOctaves", noiseSettings.numberOfOctaves);
		float maxElevation = noiseSettings.CalculateTheoreticalMaxElevation();
		shaderPtr->setFloat("maxElevation", maxElevation);
		surfaceCameraPtr->SetMaxElevationFallback(maxElevation);
	}

	if (ImGui::SliderFloat("Strength", &noiseSettings.strength, 0.0f, 2.0f))
	{
		shaderPtr->Activate();
		shaderPtr->setFloat("noise.strength", noiseSettings.strength);
		float maxElevation = noiseSettings.CalculateTheoreticalMaxElevation();
		shaderPtr->setFloat("maxElevation", maxElevation);
		surfaceCameraPtr->SetMaxElevationFallback(maxElevation);
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
		surfaceCameraPtr->SetMaxElevationFallback(maxElevation);
	}

	if (ImGui::SliderFloat("Min value", &noiseSettings.minValue, 0.0f, 2.0f))
	{
		shaderPtr->Activate();
		shaderPtr->setFloat("noise.minValue", noiseSettings.minValue);
		float maxElevation = noiseSettings.CalculateTheoreticalMaxElevation();
		shaderPtr->setFloat("maxElevation", maxElevation);
		surfaceCameraPtr->SetMaxElevationFallback(maxElevation);
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