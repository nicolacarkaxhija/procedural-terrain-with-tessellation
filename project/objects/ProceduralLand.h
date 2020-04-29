#pragma once
#include <glm/glm.hpp>
//#include <model.h>
#include "../Engine/texture.h"
//#include "TessShader.h"
#include "../Engine/shader.h"
#include <camera.h>
#include "../Engine/glError.h"
#include "renderObjectInterface.h"

enum tPosition {
	C, N, S, E, W, SE, SW, NE, NW, totTiles
};

class ProceduralLand : public renderObjectInterface
{
public:

	ProceduralLand(int gl);
	
	virtual ~ProceduralLand();
	virtual void draw();
	virtual void setGui();
	virtual void update();

	void updateTilesPositions();
	void setPositionsArray(std::vector<glm::vec2> & pos);


	glm::vec2 position, eps;
	float up = 0.0;
	
	bool inTile(Camera camera, glm::vec2 pos);
	static const int tileW = 10.*100.;

	void setOctaves(int oct) {
		if (oct > 0) octaves = oct;
	}

	void setFreq(float freq) {
		if (freq > 0.0f) { this->frequency = freq; }
	}

	void setDispFactor(float disp) {
		if (disp > 0.0f) { dispFactor = disp; }
	}

	void setScale(float scale) {
		glm::mat4 id;
		glm::mat4 scaleMatrix = glm::scale(id, glm::vec3(scale, 0.0, scale));
		glm::mat4 positionMatrix = glm::translate(id, glm::vec3(position.x*scale/this->scaleFactor, 0.0, position.y*scale / this->scaleFactor));
		modelMatrix = positionMatrix * scaleMatrix;
		scaleFactor = scale;
	}

	void setGrassCoverage(float gc) {
		grassCoverage = gc;
	}

	void setTessMultiplier(float tm) {
		if (tm > 0.0) tessMultiplier = tm;
	}

	int getOctaves() const { return octaves; }
	float getFreq() const { return frequency; }
	float getDispFactor() const { return dispFactor; }
	float getScale() const { return scaleFactor; }
	float getGrassCoverage() const { return grassCoverage; }
	float getTessMultiplier() const { return tessMultiplier; }

private:
	void deleteBuffer();
	int res;
	unsigned int planeVBO, planeVAO, planeEBO;
	unsigned int randomTexture;
	float dispFactor, scaleFactor, frequency, grassCoverage, tessMultiplier, fogFalloff, power;
	int octaves;
	int gridLength;
	glm::vec3 rockColor;

	static bool drawFog;
	unsigned int * textures, posBuffer;

	Shader * shad;
	glm::mat4 modelMatrix;

	std::vector<glm::vec2> positionVec;


	void drawVertices(int nInstances);

	void setPos(int row, int col, glm::vec2 pos) {
		positionVec[col + row * gridLength] = pos;
	}

	glm::vec2 getPos(int row, int col) {
		return positionVec[col + row * gridLength];
	}

	void generateGrid(glm::vec2 offset);
	void generateRandomTeture();
	bool getWhichTileCameraIs(glm::vec2& result);

	void getColRow(int i, int& col, int& row) {
		col = (i) % gridLength;

		row = (i - col) / gridLength;
	}

	void reset();
};

