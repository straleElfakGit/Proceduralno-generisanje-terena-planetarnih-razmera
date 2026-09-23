#include "SceneMenu.h"

SceneMenu::SceneMenu(ApplicationBase* app, Scene*& currentScenePtr) :
	Scene(app), currentScene(currentScenePtr) { }

SceneMenu::~SceneMenu() 
{
	if (currentScene != this)
		delete currentScene;
	currentScene = nullptr;
}

void SceneMenu::OnImGuiRender()
{
	for (auto& scene : scenes)
	{
		if (ImGui::Button(scene.first.c_str())) {
			currentScene = scene.second();
			currentScene->Start();
		}
	}
}

void SceneMenu::Render()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
