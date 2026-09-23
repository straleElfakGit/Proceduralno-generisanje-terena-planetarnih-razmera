#include "BasicScene.h"

BasicScene::BasicScene(ApplicationBase* app) : Scene(app) 
{ 
	shaderPtr = std::make_unique<Shader>("assets/Shaders/default.vert", "assets/Shaders/default.frag");
	cameraPtr = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 2.0f));
}

void BasicScene::Start()
{
	glEnable(GL_DEPTH_TEST);
}

void BasicScene::Update(float deltaTime)
{
	GLFWwindow* win = app->GetGLFWWindow();
	WindowData* data = (WindowData*)glfwGetWindowUserPointer(win);

	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
		cameraPtr->Inputs(win, deltaTime, data->width, data->height);
}

void BasicScene::Render()
{
	GLCall(glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w));
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	glm::mat4 model = glm::mat4(1.0f);
	shaderPtr->Activate();
	shaderPtr->setMatrix("model", model);

	GLFWwindow* win = app->GetGLFWWindow();
	WindowData* data = (WindowData*)glfwGetWindowUserPointer(win);
	cameraPtr->Matrix(fov, 0.1f, 1000.0f, *shaderPtr, "camMat", data->width, data->height);
}

void BasicScene::OnImGuiRender()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);
	{
		static float f = 0.0f;
		static int counter = 0;                        

		ImGui::Text("This is some useful text.");               
		ImGui::Checkbox("Demo Window", &show_demo_window);      
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            
		ImGui::ColorEdit3("clear color", (float*)&clear_color); 

		if (ImGui::Button("Button"))                            
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	}

	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}
}

void BasicScene::OnScroll(double xoffset, double yoffset)
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