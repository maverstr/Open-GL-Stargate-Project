// Local Headers
#include "glitter.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <iostream> 
#include <fstream>
#include <vector>
#include <errno.h>
#include "Shader.hpp"
#include "Model.hpp"
#include "Camera.hpp"
#include "LightSource.h"
#include "Jumper.hpp"
using namespace std;

//matrices
#include <glm/gtc/matrix_transform.hpp>

//audio irrKlang lib
#include <irrklang/irrKlang.h>
using namespace irrklang;
ISoundEngine* SoundEngine = createIrrKlangDevice();
bool musicBool = false;




//////////////////////////////////////////
////       FUNCTIONS DECLARATIONS      ///
//////////////////////////////////////////
// Callbacks
static bool keys[1024];
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mode*/);
static void mouse_button_callback(GLFWwindow* /*window*/, int button, int action, int /*mods*/);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double /*xoffset*/, double yoffset);

//misc
void printMatrix(glm::mat4 m);
void printVec3(glm::vec3 v);
void showFPS(void);
GLuint loadTexture(char const* path);

//VAO Creations
GLuint createAxisVAO(void);
GLuint createCubeMapVAO(void);
GLuint createCubeMapTexture(void);
GLuint createStarsVAO(int* starsCount, int maxStars = 0);
void createAsteroidVAO(int asteroidAmount, Model asteroidModel, glm::vec3 planetPos);

//Coordinate systems
glm::mat4 moveModel(Jumper jumper, bool outlining);
glm::mat4 createProjectionMatrix(void);
glm::mat4 createViewMatrix(void);
glm::mat4 createModelMatrix(void);
glm::mat4 createMVPMatrix(glm::mat4 model, glm::mat4 view, glm::mat4 projection);

//////////////////////////////////////////
////         WINDOW PARAMETERS         ///
//////////////////////////////////////////
const int windowWidth = 1690;
const int windowHeight = 1050;
const char* windowTitle = "Stargate Project";


bool translationMovement[6]; //up down left right forward backward
bool rotationMovement[6];
float transX = 0;
float transY = 0;
float transZ = 0;
float rotX = 0;
float rotY = 0;
float rotZ = 0;

//Camera
Camera camera(glm::vec3(22.0f, 16.0f, -2.0f));
float lastX = windowWidth / 2.0f;
float lastY = windowHeight / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float baseCameraMovementSpeed = camera.MovementSpeed;
float FastCameraMovementSpeed = baseCameraMovementSpeed * 3;
bool SprintActivated = false;
bool cameraMovement[6];

//lights
vector<LightSource*> lightArray; //array of pointers to all light sources. REMEMBER TO DELETE POINTERS AS I DELETE THE OBJECTS
int lightCounter = 0;

//jumper
glm::vec3 flashlightJumperOffset = glm::vec3(0.0f, -1.27f, 5.5f);
glm::vec3 jumperFirstPersonOffset = glm::vec3(0.0f, 0.0f, 5.5f);
bool jumperOutlining = true;
float timeOfExplosion = 0.0f;
float explosionDistance = 0;
float maxExplosionDistance = -1;
bool isExploded = false;

//stars
int starsCount = 0;

//planet
glm::vec3 planetPos = glm::vec3(-400.0f, -150.0f, 120.0f);

//sun
glm::vec3 sunPos = glm::vec3(-600.0f, -150.0f, -100.0f);

//asteroids
unsigned int asteroidAmount = 1000;



//Coordinate system matrix initialization
glm::mat4 modelMatrix = glm::mat4(0);
glm::mat4 viewMatrix = glm::mat4(0);
glm::mat4 projectionMatrix = glm::mat4(0);
glm::mat4 MVPMatrix = glm::mat4(0);



//////////////////////////////////////////
////   MAIN MAIN MAIN MAIN MAIN MAIN   ///
//////////////////////////////////////////
int main(int argc, char* argv[]) {
	//sound to loop during the whole game
	ISound* music = SoundEngine->play2D("audio/MF-W-90.XM", true, false, true, ESM_AUTO_DETECT, true);
	ISoundEffectControl* fx = music->getSoundEffectControl();
	fx->enableWavesReverbSoundEffect(); //adds reverb effect
	music->setIsPaused(true);

	// Load GLFW and Create a Window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	auto mWindow = glfwCreateWindow(windowWidth, windowHeight, windowTitle, nullptr, nullptr);
	// Check for Valid Context
	if (mWindow == nullptr) {
		fprintf(stderr, "Failed to Create OpenGL Context");
		return EXIT_FAILURE;
	}
	// Create Context and Load OpenGL Functions
	glfwMakeContextCurrent(mWindow);
	gladLoadGL();
	fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

	// Set the required callback functions
	glfwSetFramebufferSizeCallback(mWindow, framebuffer_size_callback);
	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(mWindow, key_callback);
	glfwSetCursorPosCallback(mWindow, mouse_callback);
	glfwSetMouseButtonCallback(mWindow, mouse_button_callback);
	glfwSetScrollCallback(mWindow, scroll_callback);

	// Change Viewport
	int width, height;
	glfwGetFramebufferSize(mWindow, &width, &height);
	glViewport(0, 0, width, height);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	glEnable(GL_PROGRAM_POINT_SIZE); //allows to modify the point size (used in stars)

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //allows some transparency
	glEnable(GL_BLEND);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); //do nothing if stencil and depth tests fail (keep values) but replace with 1's if succeed

	//Shaders
	Shader axisShader = Shader("Shaders/axis.vert", "Shaders/axis.frag");
	axisShader.compile();

	Shader skyboxShader = Shader("Shaders/skybox.vert", "Shaders/skybox.frag");
	skyboxShader.compile();

	Shader stargateShader = Shader("Shaders/model.vert", "Shaders/model.frag", "Shaders/model.geom");
	//Shader modelShader = Shader("Shaders/modelWithWholeMaterial.vert", "Shaders/modelWithWholeMaterial.frag");
	stargateShader.compile();

	Shader jumperShader = Shader("Shaders/model.vert", "Shaders/model.frag", "Shaders/model.geom");
	jumperShader.compile();

	Shader modelOutlining = Shader("Shaders/modelOutlining.vert", "Shaders/modelOutlining.frag");
	modelOutlining.compile();

	Shader planetShader = Shader("Shaders/planet.vert", "Shaders/planet.frag"); 
	planetShader.compile();

	Shader sunShader = Shader("Shaders/sun.vert", "Shaders/sun.frag");
	sunShader.compile();

	Shader asteroidShader = Shader("Shaders/asteroid.vert", "Shaders/asteroid.frag");
	asteroidShader.compile();

	Shader starsShader = Shader("Shaders/stars.vert", "Shaders/stars.frag");
	starsShader.compile();

	Shader lightShader = Shader("Shaders/lightSource.vert", "Shaders/lightSource.frag");
	lightShader.compile();

	//Textures
	GLuint skyboxTexture = createCubeMapTexture();
	GLuint jumperReflectionMap = loadTexture("Models/reflectionMapJumper.png");

	//VAO instanciation
	GLuint AxisVAO = createAxisVAO();
	GLuint skyboxVAO = createCubeMapVAO();
	GLuint starsVAO = createStarsVAO(&starsCount);
	//GLuint starsVAO = createStarsVAO(&starsCount, 100); //limit max number of stars to 100
	//Model StargateModel = Model("Models/untitled.obj"); //Stargate
	Model JumperModel = Model("Models/Jumper.obj");
	Jumper jumper1 = Jumper(&JumperModel);
	Model PlanetModel = Model("Models/planet.obj");
	Model AsteroidModel = Model("Models/rock.obj");
	createAsteroidVAO(asteroidAmount, AsteroidModel, planetPos); //no return value as there is one VAO per asteroid...
	Model SunModel = Model("Models/Sun.obj");

	//lights
	//pointer, pos, [color] OR [ambient, diffuse, specular,], [constant, linear, quadratic attenuation,] [spotlight direction, inner angle, outer angle,] size, VAO from other lights (0 if none already created)
	LightSource rotatingLight = LightSource(&lightCounter, POINTLIGHT, glm::vec3(5.0f, 2.0f, 10.0f), glm::vec3(0.9f, 0.95f, 0.4f), 1.0f, 0.082f, 0.0019f, 0.5f, 0);
	lightArray.push_back(&rotatingLight);
	GLuint lightVAO = rotatingLight.getVAO();
	
	LightSource blueLight = LightSource(&lightCounter, POINTLIGHT, glm::vec3(8.0f, 4.0f, 2.0f), glm::vec3(0.0f,0.3f,0.3f), 1.0f, 0.022f, 0.0019f, 0.5f, lightVAO);
	lightArray.push_back(&blueLight);
	LightSource flashLight = LightSource(&lightCounter, SPOTLIGHT, glm::vec3(5.0f, 2.0f, 10.0f), glm::vec3(0.9f, 0.95f, 0.4f), glm::vec3(0.9f, 0.95f, 0.4f), glm::vec3(0.9f, 0.95f, 0.4f), 1.0f, 0.022f, 0.0019f, camera.Front, 4.5f, 6.5f, 0.5f, 0);
	lightArray.push_back(&flashLight);
	LightSource jumperFlashLight = LightSource(&lightCounter, SPOTLIGHT, jumper1.Position + flashlightJumperOffset, glm::vec3(0.9f, 0.95f, 0.4f), glm::vec3(0.9f, 0.95f, 0.4f), glm::vec3(0.9f, 0.95f, 0.4f), 1.0f, 0.022f, 0.0019f, glm::vec3(0.0f,0.0f,1.0f), 8.5f, 12.5f, 0.2f, 0);
	lightArray.push_back(&jumperFlashLight);
	
	LightSource sunLight = LightSource(&lightCounter, POINTLIGHT, sunPos, glm::vec3(1.0f, 0.6f, 0.2f)*1.0f, 1.0f, 0.00080f, 0.0000070f, 1.0f, lightVAO);
	lightArray.push_back(&sunLight);

	//camera initial look at position
	camera.setInitialLookAt(glm::vec3(0.0f, 0.0f, 0.0f));
	//////////////////////////////////////////
	////           RENDERING LOOP          /// 
	//////////////////////////////////////////
	while (glfwWindowShouldClose(mWindow) == false) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//audio
		if (musicBool) {
			music->setIsPaused(!music->getIsPaused());
			musicBool = false;
		}


		//Handle movements
		jumper1.clearMovement();
		if (!isExploded) { //removes the ability to move if exploded
			if (translationMovement[0])
				jumper1.ProcessKeyboard(FORWARD, deltaTime);
			if (translationMovement[1])
				jumper1.ProcessKeyboard(BACKWARD, deltaTime);
			if (translationMovement[2])
				jumper1.ProcessKeyboard(LEFT, deltaTime);
			if (translationMovement[3])
				jumper1.ProcessKeyboard(RIGHT, deltaTime);
			if (translationMovement[4])
				jumper1.ProcessKeyboard(UP, deltaTime);
			if (translationMovement[5])
				jumper1.ProcessKeyboard(DOWN, deltaTime);

			if (rotationMovement[0])
				jumper1.ProcessKeyboard(PITCH_UP, deltaTime);
			if (rotationMovement[1])
				jumper1.ProcessKeyboard(PITCH_DOWN, deltaTime);
			if (rotationMovement[2])
				jumper1.ProcessKeyboard(ROLL_LEFT, deltaTime);
			if (rotationMovement[3])
				jumper1.ProcessKeyboard(ROLL_RIGHT, deltaTime);
			if (rotationMovement[4])
				jumper1.ProcessKeyboard(YAW_LEFT, deltaTime);
			if (rotationMovement[5])
				jumper1.ProcessKeyboard(YAW_RIGHT, deltaTime);
		}
		if (cameraMovement[0])
			camera.ProcessKeyboard(CAM_FORWARD, deltaTime);
		if (cameraMovement[1])
			camera.ProcessKeyboard(CAM_BACKWARD, deltaTime);
		if (cameraMovement[2])
			camera.ProcessKeyboard(CAM_LEFT, deltaTime);
		if (cameraMovement[3])
			camera.ProcessKeyboard(CAM_RIGHT, deltaTime);
		if (cameraMovement[4])
			camera.ProcessKeyboard(CAM_UP, deltaTime);
		if (cameraMovement[5])
			camera.ProcessKeyboard(CAM_DOWN, deltaTime);

		if (SprintActivated) {
			camera.MovementSpeed = FastCameraMovementSpeed;
		}
		else {
			camera.MovementSpeed = baseCameraMovementSpeed;
		}

		//	camera.updatePositionFPSView(jumper1.Position + +glm::vec3(jumper1.Right * jumperFirstPersonOffset.x) + glm::vec3(jumper1.Up * jumperFirstPersonOffset.y) + glm::vec3(jumper1.Front * jumperFirstPersonOffset.z), - jumper1.Front);

		showFPS();

		//Calculate coordinate systems every frame
		modelMatrix = createModelMatrix();
		viewMatrix = createViewMatrix();
		projectionMatrix = createProjectionMatrix();
		MVPMatrix = createMVPMatrix(modelMatrix, viewMatrix, projectionMatrix);

		// Background Fill Color
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glStencilMask(0x00); //makes sure we don't update stencil buffer by mistake


		//SKYBOX (could be drawn last to optimize performance by not rendering fragments that do not pass the depth test)
		glDisable(GL_CULL_FACE); //needs to be turned off here since we are inside the cube
		glDepthMask(GL_FALSE); // Remove depth writing
		skyboxShader.use();
		glm::mat4 skyboxViewMatrix = glm::mat4(glm::mat3(viewMatrix)); //remove translation component
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		skyboxShader.setInteger("skybox", 0);
		skyboxShader.setMatrix4("projection", projectionMatrix);
		skyboxShader.setMatrix4("view", skyboxViewMatrix);
		glBindVertexArray(skyboxVAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0); // draw your skybox
		glBindVertexArray(0);
		glDepthMask(GL_LESS); // Re enable depth writing

		//Axis drawing
		glBindVertexArray(AxisVAO);
		axisShader.use();
		axisShader.setMatrix4("transMat", glm::mat4(1.0f));
		axisShader.setMatrix4("MVP", MVPMatrix);
		glDrawElements(GL_LINES, 12, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	
		/*
		glDisable(GL_CULL_FACE); //needs to be turned off here since Blender model with triangles not specifically in the correct direction
		//stargate model drawing
		stargateShader.use();
		glActiveTexture(GL_TEXTURE15);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		stargateShader.setInteger("skybox", 15);
		stargateShader.setFloat("refractionRatio", 0.2f);
		stargateShader.setInteger("material.reflection", 0);
		glm::mat4 stargateModel = glm::translate(glm::mat4(1.0f), glm::vec3(-15.0f, -15.0f, -5.0f));
		stargateShader.setMatrix4("model", stargateModel);
		stargateShader.setMatrix4("view", viewMatrix);
		stargateShader.setMatrix4("projection", projectionMatrix);
		stargateShader.setVector3f("objectColor", 1.0f, 0.5f, 0.0f);
		stargateShader.setVector3f("viewPos", camera.Position);
		stargateShader.setFloat("material.shininess", 32.0f);

		stargateShader.setInteger("lightCounter", lightCounter); //Sets the number of lights in the environment
		
		for (int i = 0; i < lightCounter; i++) { //sends the light info to the object shaders
			(*lightArray[i]).setModelShaderLightParameters(stargateShader, i);
		}
		StargateModel.Draw(stargateShader);
		*/

		//Note: here the outlining will appear underneath the object drawn here -> kind of see through effect if needed !!!!

		//sun drawing
		glEnable(GL_CULL_FACE); //we can use face culling from here to save performance
		sunShader.use();
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, sunPos);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
		sunShader.setMatrix4("model", modelMatrix);
		sunShader.setMatrix4("view", viewMatrix);
		sunShader.setMatrix4("projection", projectionMatrix);
		SunModel.Draw(sunShader);
		glDisable(GL_CULL_FACE); //needs to be turned off here since Blender model with triangles not specifically in the correct direction
		glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments from this model should update the stencil buffer
		glStencilMask(0xFF); // enable writing to the stencil buffer


		//planet drawing
		glEnable(GL_CULL_FACE); //we can use face culling from here to save performance
		planetShader.use();
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, planetPos);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(8.0f, 8.0f, 8.0f));
		planetShader.setMatrix4("model", modelMatrix);
		planetShader.setMatrix4("view", viewMatrix);
		planetShader.setMatrix4("projection", projectionMatrix);
		PlanetModel.Draw(planetShader);
		glDisable(GL_CULL_FACE); //needs to be turned off here since Blender model with triangles not specifically in the correct direction
		glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments from this model should update the stencil buffer
		glStencilMask(0xFF); // enable writing to the stencil buffer
		
		// draw asteroides
		asteroidShader.use();
		asteroidShader.setMatrix4("view", viewMatrix);
		asteroidShader.setMatrix4("projection", projectionMatrix);
		asteroidShader.setInteger("texture_diffuse1", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, AsteroidModel.textures_loaded[0].id);
		for (unsigned int i = 0; i < AsteroidModel.meshes.size(); i++)
		{
			glBindVertexArray(AsteroidModel.meshes[i].VAO);
			glDrawElementsInstanced(GL_TRIANGLES, AsteroidModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, asteroidAmount);
			glBindVertexArray(0);
		}
		

		//jumper model drawing
		jumperShader.use();
		glActiveTexture(GL_TEXTURE15);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		jumperShader.setInteger("skybox", 15);
		glActiveTexture(GL_TEXTURE14);
		glBindTexture(GL_TEXTURE_2D, jumperReflectionMap);
		if (isExploded) {
			explosionDistance = sin(((glfwGetTime() - timeOfExplosion)*2 - 1) / 3.0f); //center the range and slow down the animation
			if (explosionDistance > 0.99f) {
				maxExplosionDistance = 1;
			}
			jumperShader.setFloat("explosionDistance", max(maxExplosionDistance, explosionDistance));
		}
		else {
			jumperShader.setFloat("explosionDistance", -1);
		}
		jumperShader.setInteger("material.texure_reflectionMap", 14);
		jumperShader.setFloat("refractionRatio", 0.0f);
		jumperShader.setInteger("material.reflection", 1);
		jumperShader.setMatrix4("model", moveModel(jumper1, false));
		jumperShader.setMatrix4("view", viewMatrix);
		jumperShader.setMatrix4("projection", projectionMatrix);
		jumperShader.setVector3f("objectColor", 1.0f, 0.5f, 0.0f);
		jumperShader.setVector3f("viewPos", camera.Position);
		jumperShader.setFloat("material.shininess", 32.0f);

		jumperShader.setInteger("lightCounter", lightCounter); //Sets the number of lights in the environment
		for (int i = 0; i < lightCounter; i++) { //sends the light info to the object shaders
			(*lightArray[i]).setModelShaderLightParameters(jumperShader, i);
		}
		JumperModel.Draw(jumperShader);
		

		
		//Stars drawing
		glEnable(GL_CULL_FACE); //we can use face culling from here to save performance
		glBindVertexArray(starsVAO);
		starsShader.use();
		starsShader.setMatrix4("MVP", MVPMatrix);
		glDrawArraysInstanced(GL_POINTS, 0, 1, starsCount); //uses instance drawing for the stars
		glBindVertexArray(0);
		
		//light drawing
		modelMatrix = createModelMatrix();
		lightShader.use();
		
		flashLight.updateFlashLightDirection(camera.Front);
		flashLight.updatePosition(camera.Position);
		jumperFlashLight.updateFlashLightDirection(jumper1.Front);
		jumperFlashLight.updatePosition(jumper1.Position + glm::vec3(jumper1.Right * flashlightJumperOffset.x) + glm::vec3(jumper1.Up * flashlightJumperOffset.y) + glm::vec3(jumper1.Front * flashlightJumperOffset.z));
		jumperFlashLight.draw(lightShader, modelMatrix, viewMatrix, projectionMatrix, camera.Position);
		blueLight.draw(lightShader, modelMatrix, viewMatrix, projectionMatrix, camera.Position);
		
		rotatingLight.updatePosition(glm::vec3(sin(glfwGetTime() * 0.6f) * 10.0f, cos(glfwGetTime() * 0.3f) * 7.0f, sin(glfwGetTime()) * 8.0f));
		rotatingLight.draw(lightShader, modelMatrix, viewMatrix, projectionMatrix, camera.Position);
		sunLight.draw(lightShader, modelMatrix, viewMatrix, projectionMatrix, camera.Position);


		
		//2nd render pass: outlining of the jumper
		if (jumperOutlining) {
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF); //only the parts not equal to 1 (i.e. the model itself) will be drawn so we don't overwrite the model itself
			glStencilMask(0x00); // disable writing to the stencil buffer
			glDisable(GL_DEPTH_TEST); //outline is always drawn above everything
			modelOutlining.use();
			modelOutlining.setMatrix4("model", moveModel(jumper1, true));
			modelOutlining.setMatrix4("view", viewMatrix);
			modelOutlining.setMatrix4("projection", projectionMatrix);
			JumperModel.Draw(modelOutlining);
			glStencilMask(0xFF);
			glEnable(GL_DEPTH_TEST);
		}
		
		
		// Flip Buffers and Draw
		glfwSwapBuffers(mWindow);
		glfwPollEvents();
	}
	glfwTerminate();
	return EXIT_SUCCESS;
}

GLuint createAxisVAO(void) {
	GLfloat vertices[] = {
		0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //rigin red
		100.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //X red
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,//origin green
		0.0f, 100.0f, 0.0f, 0.0f, 1.0f, 0.0f,//Y green
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,//origin blue
		0.0f, 0.0f, 100.0f, 0.0f, 0.0f, 1.0f };//Z blue
	GLuint indices[] = {
		0, 1,
		2, 3,
		4, 5 
	};

	GLuint EBO, VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //VBO

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); //EBO

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0); //position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); //colors
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0); //ici on peut !
	glBindVertexArray(0);
	return VAO;
}


//////////////////////////////////////////
////          CUBEMAP SKYBOX           ///
//////////////////////////////////////////

GLuint createCubeMapTexture(void) {
	std::vector<std::string> textures = { "CubeMap/posx.png", "CubeMap/negx.png", "CubeMap/posy.png", "CubeMap/negy.png", "CubeMap/posz.png", "CubeMap/negz.png" }; // Must be 6 images
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height, n;
	unsigned char* image;
	for (GLuint i = 0; i < textures.size(); i++) {
		image = stbi_load(textures[i].c_str(), &width, &height, &n, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		stbi_image_free(image);
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return texture;
}


GLuint createCubeMapVAO(void) {
	GLfloat cube_vertices[] = {
		// front
		-1.0, -1.0,  1.0,
		 1.0, -1.0,  1.0,
		 1.0,  1.0,  1.0,
		-1.0,  1.0,  1.0,
		// back
		-1.0, -1.0, -1.0,
		 1.0, -1.0, -1.0,
		 1.0,  1.0, -1.0,
		-1.0,  1.0, -1.0
	};

	GLushort cube_elements[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
	};
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0); //position
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return VAO;
}

GLuint createStarsVAO(int* starsCount, int maxStars) { //maxstars default to zero
	ifstream positionFile;
	string filename = "./CubeMap/StarsRandomCoords.txt";
	positionFile.open(filename);
	if (positionFile.is_open()) {
		cout << "correctly opened star file at " + filename << endl;
	}
	else {
		cout << "failed to open star file at " + filename << endl;
	}
	float starPosX, starPosY, starPosZ, starSize;
	int i = 0;
	const int arraySize = 40000; 
	GLfloat starsPositions[] = { 1.0, 1.0, 1.0, 1.0 };
	glm::vec4 starsInfos[arraySize];
	while ((positionFile >> starPosX >> starPosY >> starPosZ >> starSize )&& i < arraySize)
	{
		starsInfos[i].x = starPosX;
		starsInfos[i].y = starPosY;
		starsInfos[i].z = starPosZ;
		starsInfos[i].w = starSize;
		i++;
		if ((i >= maxStars) && (maxStars != 0))
			break; //allows to reduce the number of stars if impact on performance (shoud not because instancing !!)
	}
	*starsCount = i;
	GLuint VBO, VAO, instanceVBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(starsPositions), starsPositions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0); //position
	glEnableVertexAttribArray(0);
	glGenBuffers(1, &instanceVBO); //instance VBO, picked up from the star file
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(starsInfos), &starsInfos[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);	//set instance data
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from the instance VBO
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribDivisor(1, 1); // tell OpenGL this is an instanced vertex attribute and should send it to VS only once per instance and not once per vertex
	glBindVertexArray(0);
	return VAO;
}

void createAsteroidVAO(int asteroidAmount, Model asteroidModel, glm::vec3 planetPos) {
	//Note: largely inspired by learnopengl.com instancing tutorial
	// generate a large list of semi-random model transformation matrices
	// ------------------------------------------------------------------
	unsigned int amount = asteroidAmount;
	glm::mat4* modelMatrices;
	modelMatrices = new glm::mat4[amount];
	srand(glfwGetTime()); // initialize random seed	
	float radius = 120.0f;
	float offset = 40.0f;
	for (unsigned int i = 0; i < amount; i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)amount * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement + planetPos.x; //adding the planet pos to center the asteroid field on the planet
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.3f + planetPos.y; // keep height of asteroid field smaller compared to width of x and z
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement + planetPos.z;
		model = glm::translate(model, glm::vec3(x, y, z));

		// 2. scale: Scale between 0.05 and 0.25f
		float scale = (rand() % 20) / 100.0f + 0.05;
		model = glm::scale(model, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. now add to list of matrices
		modelMatrices[i] = model;
	}

	// configure instanced array
// -------------------------
	unsigned int VBO, VAO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

	// set transformation matrices as an instance vertex attribute (with divisor 1)
	for (unsigned int i = 0; i < asteroidModel.meshes.size(); i++)
	{
		VAO = asteroidModel.meshes[i].VAO;
		glBindVertexArray(VAO);
		// set attribute pointers for matrix (4 times vec4)
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}
}

//////////////////////////////////////////
////        COORDINATE SYSTEMS         ///
//////////////////////////////////////////
glm::mat4 moveModel(Jumper jumper, bool outlining) {
	glm::mat4 modelMat = jumper.rotMatTotal;

	modelMat[3] = glm::vec4(jumper.Position, 1.0f);
	if (outlining) {
		modelMat = glm::scale(modelMat, glm::vec3(1.2f, 1.2f, 1.2f));
	}
	return modelMat;
}


glm::mat4 createViewMatrix(void) {
	glm::mat4 ViewMat = camera.GetViewMatrix();
	return ViewMat;
}

glm::mat4 createViewMatrixAutoRotation(void) {
	float radius = 10.0f;
	float camX = sin(glfwGetTime() * 0.6f) * radius;
	float camY = cos(glfwGetTime() * 0.3f) * radius;
	float camZ = sin(glfwGetTime()) * radius;
	glm::vec3 camPos = glm::vec3(camX, camY, camZ);
	glm::vec3 camLooksAt = glm::vec3(0, 0, 0);
	glm::vec3 headOrientation = glm::vec3(0, 1, 0);
	glm::mat4 viewMat = glm::lookAt(camPos, camLooksAt, headOrientation);
	return viewMat;
}

glm::mat4 createProjectionMatrix(void) {
	float aspectRatio = (float) windowWidth / (float) windowHeight;
	float zNear = 0.1f;
	float zFar = 10000.0f;
	glm::mat4 projectionMat = glm::perspective(glm::radians(- camera.Fov), aspectRatio, zNear, zFar);
	return projectionMat;
}

glm::mat4 createModelMatrix(void) {
	glm::mat4 Model = glm::mat4(1.0f);
	return Model;
}

glm::mat4 createMVPMatrix(glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
	glm::mat4 MVP = projection * view * model;
	return MVP;
}

//////////////////////////////////////////
////           MISCELLANEOUS           ///
//////////////////////////////////////////
void showFPS(void) {
	static double lastTime = glfwGetTime();
	static int nbFrames = 0;

	// Measure speed
	double currentTime = glfwGetTime();
	nbFrames++;
	if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
										 // printf and reset timer
		std::cout << 1000.0 / double(nbFrames) << " ms/frame -> " << nbFrames << " frames/sec" << std::endl;
		nbFrames = 0;
		lastTime += 1.0;
	}
}


GLuint loadTexture(char const* path)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* imageData = stbi_load(path, &width, &height, &nrComponents, 0);
	if (imageData)
	{
		GLenum format{};
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
		glGenerateMipmap(GL_TEXTURE_2D); // generate the mipmaps

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(imageData);
		cout << "Texture loaded at path: " << path << std::endl;
	}
	else
	{
		cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(imageData);
	}
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidently mess up our texture.
	return textureID;
}

//////////////////////////////////////////
////         CALLBACKS FUNCTIONS       ///
//////////////////////////////////////////
static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mode*/)
{
	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;

	if (keys[GLFW_KEY_ESCAPE])
		glfwSetWindowShouldClose(window, GL_TRUE);

	// V-SYNC
	if (keys[GLFW_KEY_P]) {
		static bool vsync = true;
		if (vsync) {
			glfwSwapInterval(1);
		}
		else {
			glfwSwapInterval(0);
		}
		vsync = !vsync;
	}

	//translations
	translationMovement[0] = keys[GLFW_KEY_T];
	translationMovement[1] = keys[GLFW_KEY_G];
	translationMovement[2] = keys[GLFW_KEY_F];
	translationMovement[3] = keys[GLFW_KEY_H];
	translationMovement[4] = keys[GLFW_KEY_Y];
	translationMovement[5] = keys[GLFW_KEY_B];
	SprintActivated = keys[GLFW_KEY_LEFT_SHIFT];

	//rotations
	rotationMovement[0] = keys[GLFW_KEY_UP];
	rotationMovement[1] = keys[GLFW_KEY_DOWN];
	rotationMovement[2] = keys[GLFW_KEY_LEFT];
	rotationMovement[3] = keys[GLFW_KEY_RIGHT];
	rotationMovement[4] = keys[GLFW_KEY_KP_ADD];
	rotationMovement[5] = keys[GLFW_KEY_KP_SUBTRACT];

	cameraMovement[0] = keys[GLFW_KEY_W];
	cameraMovement[1] = keys[GLFW_KEY_S];
	cameraMovement[2] = keys[GLFW_KEY_A];
	cameraMovement[3] = keys[GLFW_KEY_D];
	cameraMovement[4] = keys[GLFW_KEY_SPACE];
	cameraMovement[5] = keys[GLFW_KEY_X];

	//jumper outlining
	if (keys[GLFW_KEY_O])
		jumperOutlining = !jumperOutlining;

	//jumper explosion
	if (keys[GLFW_KEY_K]) {
		if (isExploded) {
			isExploded = false; //cancels the explosion
		}
		else {
			isExploded = true;
			timeOfExplosion = glfwGetTime();
			maxExplosionDistance = -1; //resets to sin(-90)
		}
	}

	//sound
	if (keys[GLFW_KEY_COMMA])
		musicBool = true;

	//Wireframe or point mode 
	if (keys[GLFW_KEY_1] || keys[GLFW_KEY_KP_1])
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (keys[GLFW_KEY_2] || keys[GLFW_KEY_KP_2])
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (keys[GLFW_KEY_3] || keys[GLFW_KEY_KP_3])
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
}

static void mouse_button_callback(GLFWwindow* /*window*/, int button, int action, int /*mods*/) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		keys[GLFW_MOUSE_BUTTON_RIGHT] = true;
	else
		keys[GLFW_MOUSE_BUTTON_RIGHT] = false;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		keys[GLFW_MOUSE_BUTTON_LEFT] = true;
	else
		keys[GLFW_MOUSE_BUTTON_LEFT] = false;

	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
		keys[GLFW_MOUSE_BUTTON_MIDDLE] = true;
	else
		keys[GLFW_MOUSE_BUTTON_MIDDLE] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;
	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double /*xoffset*/, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


////////////DEBUGGING////////////////
void printMatrix(glm::mat4 m) {
	cout << m[0][0] << " " << m[0][1] << " " << m[0][2] << " " << m[0][3] << endl;
	cout << m[1][0] << " " << m[1][1] << " " << m[1][2] << " " << m[1][3] << endl;
	cout << m[2][0] << " " << m[2][1] << " " << m[2][2] << " " << m[2][3] << endl;
	cout << m[3][0] << " " << m[3][1] << " " << m[3][2] << " " << m[3][3] << endl;
	cout << "                 " << endl;

}

void printVec3(glm::vec3 v) {
	cout << v.x << " " << v.y << " " << v.z << endl;
}