#include "PlanetScene.h"

PlanetScene::PlanetScene(ApplicationBase* app) : Scene(app)
{
	shaderPtr = std::make_unique<Shader>("assets/Shaders/planetShader.vert", "assets/Shaders/planetShader.frag");
	cameraPtr = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));

	galaxyShaderPtr = std::make_unique<Shader>("assets/Shaders/skyBoxShader.vert", "assets/Shaders/skyBoxShader.frag");
	galaxyPtr = std::make_unique<Galaxy>(*galaxyShaderPtr);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
}

void PlanetScene::Start()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
}

void PlanetScene::UpdateSunPosition()
{
	glm::vec3 sunPosition = glm::vec3(
		std::cos(sunSettings.sunAngle) * sunSettings.sunDistance,
		sunSettings.sunHeight,
		std::sin(sunSettings.sunAngle) * sunSettings.sunDistance);

	sunSettings.lightPtr->UpdateDirection(glm::normalize(-sunPosition));
}

void PlanetScene::Update(float deltaTime)
{
	GLFWwindow* win = app->GetGLFWWindow();
	WindowData* data = (WindowData*)glfwGetWindowUserPointer(win);

	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
		cameraPtr->Inputs(win, deltaTime, data->width, data->height);

	if (sunSettings.rotatePlanet)
		sunSettings.planetAngle += sunSettings.planetVelocity * deltaTime;
	else
		sunSettings.sunAngle += sunSettings.sunVelocity * deltaTime;

	sunSettings.planetAngle = std::fmod(sunSettings.planetAngle, glm::two_pi<float>());
	sunSettings.sunAngle = std::fmod(sunSettings.sunAngle, glm::two_pi<float>());

	UpdateSunPosition();

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

	if (planetSettings.showMesh)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindVertexArray(vao);
	unsigned int res = planetSettings.resolution;
	int totalVertices = 6 * 6 * res * res;
	glDrawArrays(GL_TRIANGLES, 0, totalVertices);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void PlanetScene::RenderPlanetPropertiesGui()
{
	ImGui::Checkbox("Show Mesh", &planetSettings.showMesh);

	int currentRes = static_cast<int>(planetSettings.resolution);
	if (ImGui::SliderInt("Resolution", &currentRes, 2, 256))
		planetSettings.resolution = static_cast<unsigned int>(currentRes);

	float currentRadius = planetSettings.radius;
	if (ImGui::SliderFloat("Radius", &currentRadius, 1.0f, 10.0f))
	{
		planetSettings.radius = currentRadius;
		LOG_INFO("Radius set to {}", planetSettings.radius);
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

void PlanetScene::OnImGuiRender()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	if (ImGui::CollapsingHeader("Planet Settings"))
		RenderPlanetPropertiesGui();

	if (ImGui::CollapsingHeader("Sun & Lighting"))
		RenderSunGui();

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