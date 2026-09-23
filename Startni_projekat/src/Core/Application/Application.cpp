#include "Application.h"

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void process_input(GLFWwindow* window);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

Application::Application() : ApplicationBase() { }

Application::~Application() 
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Application::Initialize()
{
	window->SetWindowSizeCallback(framebuffer_size_callback);
	window->SetScrollCallback(scroll_callback);

	InitializeImGui();
}

void Application::InitializeImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window->GetGLFWWindow(), true);
	ImGui_ImplOpenGL3_Init(GLSL_VERSION_STR);
}

void Application::Loop()
{
	while (window->IsOpen()) {
		float dt = CalculateDeltaTime();
		GLFWwindow* rawWindow = window->GetGLFWWindow();
		process_input(rawWindow);

		Scene* activeScene = menuScene->GetActiveScene();

		if (activeScene) {
			activeScene->Update(dt);
			activeScene->Render();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			if (activeScene->HasImGui()) {
				ImGui::Begin("Scene");
				if (activeScene->IsReturnable() && ImGui::Button("<-"))
				{
					delete activeScene;
					activeScene = nullptr;
					menuScene->ResetActiveScene();
				}
				if (activeScene) {
					if (activeScene->IsReturnable()) {
						ImGui::SameLine();
						ImGui::Text("Return back to Menu.");
					}
					activeScene->OnImGuiRender();
				}
				ImGui::End();
			}

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		window->Display();
		window->PollEvents();
	}
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
	if (data) {
		data->width = width;
		data->height = height;
	}
}

static void process_input(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
	if (data && data->app) {
		data->app->OnEventScroll(xoffset, yoffset);
	}
}