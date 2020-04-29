#pragma once

#include <glm/glm.hpp>
#include "../Engine/shader.h"
#include "renderObjectInterface.h"
#include <glm/gtc/matrix_transform.hpp>


class Skybox : public renderObjectInterface
{
public:
	Skybox();
	~Skybox();

	virtual void draw();

private:
	unsigned int skyboxVAO, skyboxVBO;
	glm::mat4 model;
	Shader * shader;
};

