#include "ProceduralLand.h"
#include "globalAttribute.h"
#include <GLFW/glfw3.h>
#include "../imgui/imgui.h"
#include "../Engine/utils.h"

bool ProceduralLand::drawFog = true;

ProceduralLand::ProceduralLand(int gl)
{
	glm::mat4 id;
	glm::mat4 scaleMatrix = glm::scale(id, glm::vec3(1.0, 0.0, 1.0));
	glm::mat4 positionMatrix = glm::translate(id, glm::vec3(0., 0.0, 0.));
	modelMatrix = positionMatrix;

	//terrain rendering parameters
	octaves = 15;
	frequency = 0.005;
	tessMultiplier = 1.0;
	dispFactor = 6599.0;
	power = 1.1;
	fogFalloff = 0.92;

	posBuffer = 0;

	//load all the 
	shad = new Shader("TerrainTessShader");
	shad->attachShader("shaders/procedural.vert")
		->attachShader("shaders/procedural.tcs")
		->attachShader("shaders/procedural.tes")
		->attachShader("shaders/procedural.frag")
		->linkPrograms();

	this->gridLength = gl + (gl + 1) % 2; //ensure gridLength is odd


	//resolution of the base plane
	res = 2;
	initializePlaneVAO(res, tileW, &planeVAO, &planeVBO, &planeEBO);

	//load a bunch of textures
	this->generateRandomTeture();

	this->textures = new unsigned int[2];
	textures[0] = TextureFromFile("grass.jpg", "resources");
	textures[1] = TextureFromFile("rock.jpg", "resources");

	generateGrid(glm::vec2(0.0,0.0));

	setPositionsArray(positionVec);

	rockColor = glm::vec4(120, 105, 75, 255)*1.5f / 255.f;
}

void ProceduralLand::generateGrid(glm::vec2 offset)
{
	positionVec.clear();
	positionVec.resize(gridLength*gridLength);
	std::cout << "Grid length: " << gridLength << "; VecSize: " << sqrt(positionVec.size()) << std::endl;

	float sc = tileW;

	glm::vec2 I = glm::vec2(1, 0)*sc;
	glm::vec2 J = glm::vec2(0, 1)*sc;

	for (int i = 0; i < gridLength; i++) {
		for (int j = 0; j < gridLength; j++) {
			glm::vec2 pos = (float)(j - gridLength / 2)*glm::vec2(I) + (float)(i - gridLength / 2)*glm::vec2(J);
			setPos(i, j, pos + offset);
		}
	}
}

void ProceduralLand::generateRandomTeture() {
	//create random seed texture+
	const int w = 512;
	const int h = 512;
	const int c = 3;
	float * randomTexData = new float[w*h*c];
	for (int i = 0; i < w*h; ++i) {
		glm::vec3 ranVec = genRandomVec3() / 100.f;
		randomTexData[i*c] = ranVec.x;
		randomTexData[i*c + 1] = ranVec.y;
		randomTexData[i*c + 2] = ranVec.z;
	}
	randomTexture = TextureFromData(randomTexData, w, h, c);
	delete[] randomTexData;
}

void ProceduralLand::deleteBuffer(){
	glDeleteBuffers(1, &posBuffer);
	posBuffer = 0;
}

bool ProceduralLand::getWhichTileCameraIs(glm::vec2& result) {

	for (glm::vec2 p : positionVec) {
		if (inTile(*(scene->cam), p)) {
			result = p;
			return true;
		}
	}
	return false;
}


void ProceduralLand::draw(){
	globalAttributes* se = renderObjectInterface::scene;

	drawFog = !se->wireframe;

	glm::mat4 gWorld = modelMatrix;
	glm::mat4 gVP = se->projMatrix * se->cam->GetViewMatrix();

	shad->use();
	shad->setVec3("gEyeWorldPos", se->cam->Position);
	shad->setMat4("gWorld", gWorld);
	shad->setMat4("gVP", gVP);
	shad->setFloat("gDispFactor", dispFactor);
	shad->setVec3("u_LightColor", se->lightColor);
	shad->setVec3("u_LightPosition", se->lightPos);
	shad->setVec3("u_ViewPosition", se->cam->Position);
	shad->setVec3("fogColor", se->fogColor);
	shad->setVec3("rockColor", rockColor);
	shad->setVec3("seed", se->seed);

	shad->setInt("octaves", octaves);
	shad->setFloat("freq", frequency);
	shad->setFloat("u_grassCoverage", grassCoverage);
	shad->setFloat("tessellation_coeff", tessMultiplier);
	shad->setFloat("fogFalloff", fogFalloff*1.e-6);
	shad->setFloat("power", power);

	shad->setBool("normals", true);
	shad->setBool("drawFog", ProceduralLand::drawFog);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	shad->setInt("grass", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	shad->setInt("rock", 3);
	
	shad->setSampler2D("randomTexture", randomTexture, 7);

	int nIstances = gridLength*gridLength;

	drawVertices(nIstances);
}

void ProceduralLand::setGui()
{
	ImGui::Begin("Procedural terrain controls: ");

	ImGui::SliderFloat("Height factor", &dispFactor, 0.0f, std::pow(32.f*32.f*32.f, 1 / power));
	ImGui::SliderInt("Octaves", &octaves, 1, 20);
	if (ImGui::SliderInt("Grid length", &gridLength, 1, 500)) {
		gridLength = gridLength + (gridLength + 1) % 2; //ensure gridLength is odd
		int oldGridLength = sqrt(positionVec.size());
		glm::vec2 currentCenter = getPos(oldGridLength / 2, oldGridLength / 2);
		generateGrid(currentCenter);
	}
	ImGui::SliderFloat("Frequency",&frequency, 0.0f, 0.05f);
	ImGui::SliderFloat("Tessellation multiplier", &tessMultiplier, 0.1f, 5.f);
	ImGui::SliderFloat("Fog decay", &fogFalloff, 0.0f, 10.);
	ImGui::SliderFloat("Power filter", &power, 0.0f, 10.);

	ImGui::End();
}

void ProceduralLand::drawVertices(int nInstances) {
	glBindVertexArray(planeVAO);
	shad->use();
	glDrawElementsInstanced(GL_PATCHES, (res-1)*(res-1)*2*3, GL_UNSIGNED_INT, 0, nInstances);
	glBindVertexArray(0);
}

// Set tile position buffer by calling OpenGL API  
void ProceduralLand::setPositionsArray(std::vector<glm::vec2> & pos) {
	if (posBuffer) {
		this->deleteBuffer();
	}

	// vertex Buffer Object
	glGenBuffers(1, &posBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
	glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec2), &pos[0], GL_STATIC_DRAW);

	glBindVertexArray(planeVAO);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

	glVertexAttribDivisor(3, 1);
	glBindVertexArray(0);
	
}

bool ProceduralLand::inTile(const Camera camera, glm::vec2 pos) {
	float camX = camera.Position.x;
	float camY = camera.Position.z;

	float x = pos.x;
	float y = pos.y;

	bool inX = false;
	if ((camX <= x + 1.0 * tileW/2.0f) && (camX >= x - 1.0 * tileW/2.0f)) { inX = true; }
	bool inY = false;
	if ((camY <= y + 1.0 * tileW/2.0f) && (camY >= y - 1.0 * tileW/2.0f)) { inY = true; }

	bool result = inX && inY;
	return result;

}


ProceduralLand::~ProceduralLand()
{

}

void ProceduralLand::update() {
	this->updateTilesPositions();
}

void ProceduralLand::updateTilesPositions() {
	globalAttributes* se = renderObjectInterface::scene;
	glm::vec2 camPosition(se->cam->Position.x, se->cam->Position.z);

	glm::vec2 currentTile;
	if (getWhichTileCameraIs(currentTile)) {
		glm::vec2 center = getPos(gridLength / 2, gridLength / 2);
		for (glm::vec2& p : positionVec) {
			p += currentTile - center;
		}
		setPositionsArray(positionVec);
	}
}


void ProceduralLand::reset() {
	int octaves = this->getOctaves();
	float freq = this->getFreq();
	float grassCoverage = this->getGrassCoverage();
	float dispFactor = this->getDispFactor();
	float tessMultiplier = this->getTessMultiplier();

	setOctaves(octaves);
	setFreq(freq);
	setGrassCoverage(grassCoverage);
	setDispFactor(dispFactor);
	setTessMultiplier(tessMultiplier);
}
