#include "ApplicationBase.h"

ApplicationBase::ApplicationBase()
{
	lastFrameTime = 0.0f;
	
	WindowData windowData;

	windowData.title = "Application";
	windowData.width = 800;
	windowData.height = 600;
	windowData.VSync = true;
	windowData.app = this;

	window = Window::Create(this);
}

ApplicationBase::~ApplicationBase() 
{
	if (menuScene != nullptr)
		delete menuScene;
	menuScene = nullptr;
}

void ApplicationBase::Run()
{
	Initialize();
	Loop();
}

float ApplicationBase::CalculateDeltaTime()
{
	const float curTime = static_cast<float>(glfwGetTime());
	const Timestep dt(curTime - lastFrameTime);
	lastFrameTime = curTime;
	return dt;
}

void ApplicationBase::OnEventScroll(double xoffset, double yoffset)
{
	Scene* activeScene = menuScene->GetActiveScene();
	if (activeScene) {
		activeScene->OnScroll(xoffset, yoffset);
	}
}

void ApplicationBase::SetMenuScene(SceneMenu* menu)
{
	this->menuScene = menu;
	this->menuScene->ResetActiveScene();
}
