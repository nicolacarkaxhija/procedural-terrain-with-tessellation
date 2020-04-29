#ifndef SCENELEMENTS_H
#define SCENELEMENTS_H

#include <camera.h>
#include <glm/glm.hpp>
#include <random>

struct globalAttributes {

	glm::vec3 lightPos, lightColor, lightDir, fogColor, seed;
	glm::mat4 projMatrix;
	Camera * cam;
	bool wireframe = false;
};

#endif