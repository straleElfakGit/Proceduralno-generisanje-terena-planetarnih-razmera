#include "Core/Application/Application.h"
#include "Core/Scenes/BasicScene.h"
#include "Core/Scenes/SceneMenu.h"
#include "Core/Scenes/PlanetScene.h"
#include "Logging/Logger.h"
#include <memory>

int main() 
{	
	Logger::Init();

	LOG_FUNC();
	LOG_INFO("Startujem program");

	ApplicationBase* app = new Application();
	
	Scene* currentScene = nullptr;
	SceneMenu* menu = new SceneMenu(app, currentScene);

	menu->RegisterScene<BasicScene>("Basic Scene", app);
	menu->RegisterScene<PlanetScene>("Planet Scene", app);

	app->SetMenuScene(menu);
	app->Run();

	delete app;
	return 0;
}
