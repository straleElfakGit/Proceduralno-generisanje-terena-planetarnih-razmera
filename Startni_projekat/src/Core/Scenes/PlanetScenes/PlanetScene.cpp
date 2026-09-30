#include "PlanetScene.h"

PlanetScene::PlanetScene(ApplicationBase* app) : PlanetSceneBase(app)
{
	ResourceManager& rm = ResourceManager::GetInstance();

	shaderPtr = rm.GetOrLoadShader("PlanetShader", "assets/Shaders/simplePlanetShader.vert", "assets/Shaders/simplePlanetShader.frag");

	decoyCameraPtr = std::make_unique<DecoyCamera>();
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

void PlanetScene::StartSpecific()
{
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

	decoyCameraPtr->SetRadius(planetSettings.radius * 1.25f);
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

void PlanetScene::UpdateSpecific(float deltaTime)
{
	GLFWwindow* win = app->GetGLFWWindow();
	WindowData* data = (WindowData*)glfwGetWindowUserPointer(win);

	decoyCameraPtr->Update(deltaTime);

	shaderPtr->Activate();
	shaderPtr->setInt("maxLevel", planetQuadTree.GetSettings().maxLevel);


	glm::mat4 planetTransform = glm::mat4(1.0f);
	planetTransform = glm::rotate(planetTransform, sunSettings.planetAngle, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::vec3 activeCamWorldPos;
	glm::mat4 activeViewMat;
	glm::mat4 activeProjMat;
	float activeFov;

	if (decoyCameraPtr->IsActive())
	{
		activeCamWorldPos = decoyCameraPtr->GetPosition();
		activeViewMat = decoyCameraPtr->GetViewMatrix();
		activeProjMat = decoyCameraPtr->GetProjectionMatrix(data->width, data->height);
		activeFov = decoyCameraPtr->GetFOV();
	}
	else
	{
		activeCamWorldPos = cameraPtr->GetPosition();
		activeViewMat = cameraPtr->GetViewMatrix();
		activeProjMat = cameraPtr->GetProjectionMatrix(fov, 0.1f, 1000.0f, data->width, data->height);
		activeFov = fov;
	}

	const glm::vec3 cameraLocal = glm::vec3(glm::inverse(planetTransform) * glm::vec4(activeCamWorldPos, 1.0f));

	PlanetView view;
	view.cameraInLocalPlanetSpace = cameraLocal;
	view.localToClip = activeProjMat * activeViewMat * planetTransform;

	planetQuadTree.Select(view);
	UploadSelection(planetQuadTree.GetSelection());
}

void PlanetScene::RenderSpecific()
{
	GLFWwindow* win = app->GetGLFWWindow();
	WindowData* data = (WindowData*)glfwGetWindowUserPointer(win);

	glm::mat4 viewMat = cameraPtr->GetViewMatrix();
	glm::mat4 projectionMat = cameraPtr->GetProjectionMatrix(fov, 0.1f, 1000.0f, data->width, data->height);

	if (planetSettings.showMesh)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindVertexArray(gridVAO);
	const GLsizei instanceCount = static_cast<GLsizei>(planetQuadTree.GetSelection().size());
	if (instanceCount > 0)
		glDrawElementsInstanced(GL_TRIANGLES, gridIndexCount, GL_UNSIGNED_INT, nullptr, instanceCount);

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
		planetQuadTree.Configure(qts);
	}
}

void PlanetScene::RenderGuiSpecific()
{
	if (ImGui::CollapsingHeader("Culling & Decoy Camera Debug"))
		decoyCameraPtr->RenderGui();
}