#ifndef GLAD_H
#define GLAD_H
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

#include "Engine/Window.h"
#include "Engine/shader.h"
#include "Engine/ScreenSpaceShader.h"
#include "Engine/texture.h"

#include "objects/ProceduralLand.h"
#include "objects/Skybox.h"

#include <camera.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include "Engine/glError.h"

#include "objects/globalAttribute.h"
#include "objects/renderObjectInterface.h"
#include "objects/UserInterface.h"

#include <iostream>
#include <vector>
#include <functional>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Engine/utils.h"
#include "Engine/texture.h"

int main()
{

	// camera and window setup
	glm::vec3 startPosition(0.0f, 12000.0f, 0.0f);
	Camera camera(startPosition);

	int success;
	Window window(success, 1600, 900);
	if (!success) return -1;

	//Window class needs camera address to perform input handling
	window.camera = &camera;

	UserInterface gui(window);

	glm::vec3 fogColor(73,73,73);
	fogColor *=  1. /255.0;
	glm::vec3 lightColor(204, 205, 149);
	lightColor *= 1./255.0;

	glm::vec3 lightPosition, seed;
	glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), (float)Window::SCR_WIDTH / (float)Window::SCR_HEIGHT, 5.f, 10000000.0f);
	glm::vec3 lightDir = glm::vec3(-.5, 0.5, 1.0);

	//Every scene object need these global informations to be rendered
	globalAttributes scene;
	scene.lightPos = lightPosition;
	scene.lightColor = lightColor;
	scene.fogColor = fogColor;
	scene.seed = seed;
	scene.projMatrix = proj;
	scene.cam = &camera;
	scene.lightDir = lightDir;

	renderObjectInterface::scene = &scene;

	int gridLength = 120;
	ProceduralLand terrain(gridLength);

	Skybox skybox;
	
	gui.subscribe(&terrain)
		.subscribe(&skybox);

	//ScreenSpaceShader PostProcessing("shaders/post_processing.frag");
	ScreenSpaceShader fboVisualizer("shaders/visualizeFbo.frag");

	while (window.continueLoop())
	{
		scene.lightDir = glm::normalize(scene.lightDir);
		scene.lightPos = scene.lightDir*1e6f + camera.Position;

		// input
		float frametime = 1 / ImGui::GetIO().Framerate;
		window.processInput(frametime);

		//update tiles position to make the world infinite, clouds weather map and sky colors
		terrain.update();
		gui.update();
		skybox.update();

		// enable gamma correction, depth test and backface culling
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glClearColor(fogColor[0], fogColor[1], fogColor[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// toggle/untoggle wireframe mode
		if (scene.wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// Camera (View Matrix) setting
		glm::mat4 view = scene.cam->GetViewMatrix();
		scene.projMatrix = glm::perspective(glm::radians(camera.Zoom), (float)Window::SCR_WIDTH / (float)Window::SCR_HEIGHT, 5.f,10000000.0f);

		terrain.draw();
		skybox.draw();

		// Texture visualizer //for debugging purposes
		Shader& fboVisualizerShader = fboVisualizer.getShader();
		fboVisualizerShader.use();
		fboVisualizerShader.setSampler2D("fboTex", 0, 0);
		//fboVisualizer.draw(); 

		gui.draw();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		window.swapBuffersAndPollEvents();
	}
}
