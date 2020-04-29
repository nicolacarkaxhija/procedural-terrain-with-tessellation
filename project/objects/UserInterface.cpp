#include "UserInterface.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

#include "../Engine/utils.h"

UserInterface::UserInterface(Window& window)
{
	// GUI
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 130");
}

void UserInterface::draw()
{

	globalAttributes& scene = *this->scene;

	for (renderObjectInterface* obj : subscribers) {
		obj->setGui();
	}

	ImGui::Begin("Various controls: ");
	ImGui::DragFloat3("Light Position", &scene.lightDir[0], 0.01, -1.0, 1.0);
	ImGui::ColorEdit3("Global light color", (float*)&scene.lightColor);
	ImGui::SliderFloat("Camera speed", &scene.cam->MovementSpeed, 0.0, SPEED*3.0);




	ImGui::Checkbox("Enable/disable wireframe", &scene.wireframe);

	if (ImGui::Button("New seed"))
		scene.seed = genRandomVec3();
	ImGui::SameLine();
	if (ImGui::Button("Default seed"))
		scene.seed = glm::vec3(0.0, 0.0, 0.0);


	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();


	//actual drawing
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UserInterface::update()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

UserInterface & UserInterface::subscribe(renderObjectInterface * subscriber)
{
	subscribers.push_back(subscriber);

	return *this;
}


UserInterface::~UserInterface()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
