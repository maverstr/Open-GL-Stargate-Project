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

//matrices
#include <glm/gtc/matrix_transform.hpp>

using namespace std;


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
void showFPS(void);
GLuint loadTexture(char const* path);

//VAO Creations
GLuint createAxisVAO(void);
GLuint createCubeMapVAO(void);
GLuint createCubeMapTexture(void);
GLuint createStarsVAO();

//Coordinate systems
glm::mat4 translateModel(void);
glm::mat4 createProjectionMatrix(void);
glm::mat4 createViewMatrix(void);
glm::mat4 createModelMatrix(void);
glm::mat4 createMVPMatrix(glm::mat4 model, glm::mat4 view, glm::mat4 projection);

//////////////////////////////////////////
////         WINDOW PARAMETERS         ///
//////////////////////////////////////////
const int windowWidth = 800;
const int windowHeight = 600;
const char* windowTitle = "Stargate Project";


bool translationMovement[6]; //up down left right forward backward
float transX = 0;
float transY = 0;
float transZ = 0;

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

//Coordinate system matrix initialization
glm::mat4 modelMatrix = glm::mat4(0);
glm::mat4 viewMatrix = glm::mat4(0);
glm::mat4 projectionMatrix = glm::mat4(0);
glm::mat4 MVPMatrix = glm::mat4(0);



//////////////////////////////////////////
////   MAIN MAIN MAIN MAIN MAIN MAIN   ///
//////////////////////////////////////////
int main(int argc, char* argv[]) {
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

	glEnable(GL_PROGRAM_POINT_SIZE);

	//Shaders
	Shader axisShader = Shader("Shaders/axis.vert", "Shaders/axis.frag");
	axisShader.compile();

	Shader skyboxShader = Shader("Shaders/skybox.vert", "Shaders/skybox.frag");
	skyboxShader.compile();

	Shader modelShader = Shader("Shaders/model.vert", "Shaders/model.frag");
	//Shader modelShader = Shader("Shaders/modelWithWholeMaterial.vert", "Shaders/modelWithWholeMaterial.frag");
	modelShader.compile();

	Shader starsShader = Shader("Shaders/stars.vert", "Shaders/stars.frag");
	starsShader.compile();

	Shader lightShader = Shader("Shaders/lightSource.vert", "Shaders/lightSource.frag");
	lightShader.compile();

	//Textures
	GLuint skyboxTexture = createCubeMapTexture();
	//GLuint diffuseMap = loadTexture("Models/Material Base Color.png");
	GLuint specularMap = loadTexture("Models/specularMap.png");
	GLuint emissionMap = loadTexture("Models/emissionMap.png");
	modelShader.use();
	//modelShader.setInteger("material.diffuseMap", 5);
	modelShader.setInteger("material.texure_specular1", 6);
	modelShader.setInteger("material.emissionMap", 7);

	//VAO instanciation
	GLuint AxisVAO = createAxisVAO();
	GLuint skyboxVAO = createCubeMapVAO();
	GLuint starsVAO = createStarsVAO();
	Model mainModel = Model("Models/untitled.obj");

	//lights
	//pointer, pos, [color] OR [ambient, diffuse, specular,], [constant, linear, quadratic attenuation,] [spotlight direction, inner angle, outer angle,] size, VAO from other lights (0 if none already created)
	LightSource rotatingLight = LightSource(&lightCounter, POINTLIGHT, glm::vec3(5.0f, 2.0f, 10.0f), glm::vec3(0.9f, 0.95f, 0.4f), 1.0f, 0.082f, 0.0019f, 0.5f, 0);
	lightArray.push_back(&rotatingLight);
	GLuint lightVAO = rotatingLight.getVAO();
	LightSource blueLight = LightSource(&lightCounter, POINTLIGHT, glm::vec3(8.0f, 4.0f, 2.0f), glm::vec3(0.0f,0.3f,0.3f), 1.0f, 0.022f, 0.0019f, 0.5f, lightVAO);
	lightArray.push_back(&blueLight);
	LightSource flashLight = LightSource(&lightCounter, SPOTLIGHT, glm::vec3(5.0f, 2.0f, 10.0f), glm::vec3(0.9f, 0.95f, 0.4f), glm::vec3(0.9f, 0.95f, 0.4f), glm::vec3(0.9f, 0.95f, 0.4f), 1.0f, 0.022f, 0.0019f, camera.Front, 4.5f, 6.5f, 0.5f, 0);
	lightArray.push_back(&flashLight);

	//camera initial look at position
	camera.setInitialLookAt(glm::vec3(0.0f, 0.0f, 0.0f));
	//////////////////////////////////////////
	////           RENDERING LOOP          /// 
	//////////////////////////////////////////
	while (glfwWindowShouldClose(mWindow) == false) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//Handle movements
		transX += translationMovement[0] * 0.05f;
		transX -= translationMovement[1] * 0.05f;
		transY += translationMovement[2] * 0.05f;
		transY -= translationMovement[3] * 0.05f;
		transZ += translationMovement[4] * 0.05f;
		transZ -= translationMovement[5] * 0.05f;

		if (cameraMovement[0])
			camera.ProcessKeyboard(FORWARD, deltaTime);
		if (cameraMovement[1])
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (cameraMovement[2])
			camera.ProcessKeyboard(LEFT, deltaTime);
		if (cameraMovement[3])
			camera.ProcessKeyboard(RIGHT, deltaTime);
		if (cameraMovement[4])
			camera.ProcessKeyboard(UP, deltaTime);
		if (cameraMovement[5])
			camera.ProcessKeyboard(DOWN, deltaTime);

		if (SprintActivated) {
			camera.MovementSpeed = FastCameraMovementSpeed;
		}
		else {
			camera.MovementSpeed = baseCameraMovementSpeed;
		}

		//showFPS();
		//Calculate coordinate systems every frame
		modelMatrix = createModelMatrix();
		viewMatrix = createViewMatrix();
		projectionMatrix = createProjectionMatrix();
		MVPMatrix = createMVPMatrix(modelMatrix, viewMatrix, projectionMatrix);

		// Background Fill Color
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		
		//SKYBOX (could be drawn last to optimize performance by not rendering fragments that do not pass the depth test)
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
		
		//model drawing
		modelShader.use();
		modelShader.setMatrix4("model", translateModel());
		modelShader.setMatrix4("view", viewMatrix);
		modelShader.setMatrix4("projection", projectionMatrix);
		modelShader.setMatrix4("MVP", MVPMatrix);
		modelShader.setVector3f("objectColor", 1.0f, 0.5f, 0.0f);
		modelShader.setVector3f("viewPos", camera.Position);
		modelShader.setFloat("material.shininess", 32.0f);

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, specularMap);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, emissionMap);
		modelShader.setInteger("lightCounter", lightCounter); //Sets the number of lights in the environment
		
		for (int i = 0; i < lightCounter; i++) { //sends the light info to the object shaders
			(*lightArray[i]).setModelShaderLightParameters(modelShader, i);
		}
		mainModel.Draw(modelShader);
		
		//todo material drawing + shadow 

		//Stars drawing
		glBindVertexArray(starsVAO);
		starsShader.use();
		starsShader.setMatrix4("MVP", MVPMatrix);
		glDrawArrays(GL_POINTS, 0, 900);
		glBindVertexArray(0);

		//light drawing
		flashLight.updateFlashLightDirection(camera.Front);
		flashLight.updatePosition(camera.Position);
		rotatingLight.updatePosition(glm::vec3(sin(glfwGetTime() * 0.6f) * 10.0f, cos(glfwGetTime() * 0.3f) * 7.0f, sin(glfwGetTime()) * 8.0f));
		rotatingLight.draw(lightShader, modelMatrix, viewMatrix, projectionMatrix);
		blueLight.draw(lightShader, modelMatrix, viewMatrix, projectionMatrix);

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

GLuint createStarsVAO(void) {
		GLfloat starsPositions[] = {
		45.9, 36.6, -42.2,
	-20.4, 46.4, 9.8,
	-18.3, 7.5, 23.3,
	27.7, -29.3, -13.6,
	37.9, 26.0, 39.6,
	25.9, 15.2, 30.4,
	-1.6, -14.4, 24.4,
	-25.5, 43.8, 11.6,
	-5.4, 46.6, -34.5,
	-44.3, -33.8, 38.2,
	-44.3, 9.7, 48.4,
	16.7, 21.0, -37.4,
	-37.4, 27.3, 13.6,
	-28.6, -47.3, 27.2,
	-14.7, 44.0, -3.2,
	-34.4, 20.1, -27.8,
	-7.1, 46.4, -37.7,
	-38.8, -10.5, -26.7,
	-13.4, -22.4, -3.8,
	-3.9, -24.0, -20.6,
	-0.4, -49.9, -46.8,
	-24.7, -16.0, 25.4,
	-25.0, -40.2, 27.1,
	1.5, -2.9, -30.0,
	-15.8, 34.0, 32.5,
	-38.4, -29.6, 7.6,
	-23.1, 44.8, -25.2,
	-31.4, -49.1, 18.9,
	-34.8, 47.8, 14.7,
	-22.2, -32.5, -33.3,
	26.6, -30.6, -4.1,
	-17.0, 34.8, -6.8,
	21.0, 3.1, -39.1,
	-39.8, -27.8, 33.8,
	-16.8, 47.5, -31.0,
	15.8, 3.8, 32.6,
	32.0, -24.6, 27.0,
	5.3, -10.4, 9.2,
	-18.3, 43.4, 17.5,
	1.3, 29.6, 38.9,
	-24.6, -3.8, -26.7,
	-19.4, 1.3, -29.3,
	-3.4, 8.6, -25.4,
	-41.3, 26.1, 36.7,
	11.7, -38.2, -40.1,
	-36.3, -27.3, 31.3,
	-22.2, -7.8, -35.1,
	18.5, -32.7, 0.9,
	31.5, -28.9, 8.0,
	38.5, 26.6, -29.1,
	-22.3, 24.0, -31.6,
	48.9, 24.9, -31.7,
	3.5, 37.8, -46.7,
	-12.6, 21.6, 48.4,
	-7.2, -1.4, 33.6,
	-25.3, 37.7, -3.1,
	32.7, -22.0, -45.5,
	15.0, 7.8, 18.1,
	-38.6, -46.2, -37.9,
	-33.5, 32.7, 40.0,
	28.4, 20.6, -26.7,
	15.3, 21.9, -24.2,
	-36.4, 15.8, 6.3,
	42.4, -35.4, -7.3,
	7.5, -20.9, 28.2,
	-24.1, -14.7, 5.5,
	37.5, -12.6, 14.1,
	29.3, 40.1, -9.6,
	-28.6, 34.6, 35.2,
	-11.8, -5.6, -19.0,
	-16.7, -7.2, -25.1,
	-26.0, 20.7, -10.6,
	-49.6, 21.5, -35.9,
	-41.4, -21.0, 40.4,
	35.8, 12.9, 30.1,
	26.5, -0.6, -10.2,
	7.6, 42.7, -28.1,
	49.9, -43.0, 27.7,
	16.7, -43.7, 3.5,
	-16.3, -12.4, -39.3,
	44.0, 41.6, -16.3,
	-11.4, 28.8, -13.8,
	-15.2, -38.6, -7.3,
	-29.7, 24.3, 30.3,
	-49.0, -30.3, -16.1,
	-2.8, 42.1, 14.2,
	-29.6, 37.4, 17.0,
	-43.5, -13.4, -24.4,
	30.6, 32.7, -31.5,
	20.7, 11.4, 1.0,
	47.3, 49.9, -10.7,
	45.3, -27.9, 22.2,
	-17.0, 16.2, -42.1,
	33.2, -13.9, 5.0,
	-33.1, -23.0, 34.8,
	26.4, -32.7, -13.6,
	-46.3, -38.4, 12.0,
	-33.2, 44.0, 11.8,
	30.1, -5.5, 11.3,
	32.8, 22.6, -19.4,
	-9.1, -30.1, 22.1,
	-29.2, -21.6, -12.2,
	10.4, 35.0, 3.2,
	-18.0, -40.0, 42.0,
	-14.0, -31.6, 39.6,
	-10.7, 13.1, -38.2,
	-33.7, -12.5, 12.5,
	-29.4, -23.2, 35.4,
	32.5, -23.0, -21.2,
	35.8, 28.4, 46.8,
	19.2, 7.9, -31.9,
	-20.4, -20.0, -16.8,
	23.7, 37.9, -47.1,
	-20.1, -7.5, -43.9,
	-13.2, 19.4, 15.5,
	40.6, 32.8, 45.2,
	-18.7, 1.6, -38.7,
	-35.1, 30.0, -29.0,
	17.8, -42.3, -32.0,
	5.6, -7.7, 10.3,
	20.8, 38.5, -44.6,
	2.5, 23.1, 27.1,
	16.4, -38.8, -34.2,
	28.5, -34.5, -1.3,
	-41.1, 5.5, 44.9,
	-14.4, 3.5, -22.2,
	-13.1, 38.0, 24.8,
	27.9, -46.3, 34.3,
	21.4, -22.4, 14.3,
	-13.2, -48.5, -5.8,
	-8.0, -28.6, 0.9,
	-43.5, -48.0, -47.8,
	34.9, 47.2, -19.0,
	-4.0, 34.5, 19.0,
	-47.3, -29.9, 38.7,
	38.7, 49.2, 44.8,
	-43.3, 48.4, -1.1,
	-42.9, -41.2, -37.0,
	12.2, -38.2, -33.2,
	45.3, -48.0, 44.6,
	-23.6, -20.1, -36.9,
	-1.1, -25.2, 8.4,
	7.4, -41.1, -15.7,
	-38.3, -16.6, 41.8,
	-33.0, 12.1, -36.3,
	-9.5, 6.3, -45.1,
	45.8, -28.5, 32.0,
	-34.0, -16.2, -20.7,
	42.6, 24.4, 36.7,
	-41.3, 28.1, 49.0,
	43.7, -26.6, 41.6,
	-45.7, -42.6, -38.6,
	-36.8, -36.3, 25.0,
	-45.0, 31.0, -1.4,
	31.7, 25.6, 40.8,
	6.9, -25.8, 30.0,
	31.7, 41.3, 46.0,
	-5.0, 45.1, 47.0,
	-44.8, 22.6, 6.9,
	-31.2, 29.1, -47.6,
	-38.2, -41.1, -10.3,
	-41.1, -8.1, 5.0,
	-47.1, 33.1, 11.7,
	9.7, -24.5, -42.4,
	-39.1, 25.3, -25.6,
	-8.2, 41.2, 28.1,
	9.7, 9.2, 48.7,
	-6.4, 49.9, 21.4,
	18.4, -27.4, 36.2,
	33.3, -15.9, 13.0,
	-5.5, -13.3, 41.5,
	-29.0, -49.8, 7.8,
	-48.3, 5.9, -9.8,
	48.9, -3.4, 20.7,
	-35.7, 35.3, -19.3,
	-7.5, 15.0, 10.8,
	-46.8, 19.6, -31.9,
	18.9, 8.5, -20.7,
	29.8, -27.4, -12.3,
	40.6, 2.2, 26.1,
	41.8, 36.1, 26.6,
	18.5, -37.5, 41.7,
	-44.3, -13.6, -39.5,
	-47.8, 20.5, 0.7,
	-47.0, -35.5, 23.1,
	-35.8, 8.0, -37.1,
	36.7, -19.4, 11.0,
	-20.4, -1.2, 25.6,
	-46.0, -24.6, 3.3,
	-1.8, 19.0, -25.5,
	34.3, -22.8, -24.9,
	-19.2, -1.5, 35.6,
	-26.3, 10.8, -17.0,
	-28.4, -27.1, 26.4,
	11.2, 43.4, 27.3,
	-43.4, 25.1, -18.8,
	44.8, -24.3, -0.2,
	-49.3, -35.4, 17.5,
	36.8, 9.7, 37.1,
	-38.2, -32.5, -12.2,
	-9.9, 4.4, -26.3,
	34.2, 2.7, 10.6,
	-7.8, 44.5, -41.9,
	-48.9, 5.9, -13.8,
	-35.9, -38.8, -36.6,
	44.4, 30.1, 37.8,
	-25.3, -20.0, 23.0,
	-32.9, -12.1, -9.1,
	12.5, -6.3, -9.9,
	-41.8, -23.3, 29.5,
	-28.3, 10.7, -19.7,
	-40.5, -23.0, -6.3,
	-29.4, -48.3, -12.8,
	31.4, -5.9, -0.6,
	-49.8, 19.7, -31.1,
	-39.9, -39.6, 5.8,
	44.3, 23.4, -21.9,
	6.1, 19.0, 19.2,
	-4.9, -33.5, 24.3,
	22.9, 46.9, 33.8,
	33.2, 34.2, 32.1,
	-4.8, -13.4, 18.6,
	-32.0, 26.3, 4.4,
	26.2, 5.9, -1.6,
	-15.1, 42.1, 4.4,
	45.4, -32.0, -48.1,
	-28.9, -5.7, -23.0,
	39.7, -46.7, 24.2,
	38.7, -48.8, -13.4,
	26.1, -35.9, -12.6,
	1.0, 42.0, 25.0,
	18.9, 17.3, 7.7,
	18.7, -9.4, -13.2,
	25.7, -42.6, 47.4,
	-20.6, 11.8, 2.9,
	-28.1, -2.0, -35.9,
	28.9, -37.5, 42.9,
	13.1, 18.0, -30.3,
	-41.3, 37.5, -4.0,
	-42.1, -28.0, -21.3,
	6.0, 7.8, -34.9,
	-27.0, -39.2, 18.8,
	-32.5, 21.8, -2.4,
	6.7, -45.7, -26.5,
	36.5, 48.3, 5.6,
	-42.9, 45.8, -38.3,
	41.7, -1.7, -45.7,
	-2.1, -21.6, -24.6,
	5.7, -21.0, -41.9,
	42.1, -45.0, -1.0,
	-22.8, -7.9, 43.4,
	-48.8, -22.2, 38.1,
	-24.8, 25.9, -18.9,
	-17.8, -9.4, -22.7,
	-44.5, -41.9, 18.9,
	48.1, 15.1, 22.4,
	27.1, -36.3, 38.2,
	43.9, -40.9, -26.9,
	-34.4, 17.0, 32.0,
	-27.3, -12.7, -46.3,
	-31.2, 3.6, 33.0,
	-16.9, 30.0, -43.3,
	9.2, -28.0, 19.1,
	-28.8, 20.4, -38.6,
	-36.8, -44.8, 42.7,
	2.7, -28.1, 35.6,
	-34.8, -6.5, -9.1,
	-30.8, -28.9, -5.9,
	-39.2, 37.5, 13.6,
	17.7, -23.4, 3.0,
	-27.3, 41.2, -21.1,
	-28.2, 36.1, -47.3,
	-12.5, 35.9, 2.7,
	12.5, -28.8, 48.2,
	-39.5, -41.5, -16.1,
	28.3, 27.2, -8.8,
	-21.8, 40.9, 49.4,
	45.1, -22.4, 4.2,
	48.7, -15.7, 43.4,
	-9.8, -40.1, -25.6,
	1.3, 18.1, -4.9,
	2.1, 19.6, -32.7,
	-31.8, 40.4, 45.5,
	-48.7, -42.5, 4.7,
	9.2, 8.7, 47.0,
	-20.0, -25.3, -5.9,
	13.5, 23.6, 0.5,
	-9.0, -44.1, -23.9,
	-13.5, -4.9, -10.2,
	-42.7, 42.9, -9.9,
	38.9, -42.7, -29.9,
	-20.2, -16.2, -47.8,
	-21.7, 11.9, 6.8,
	-48.0, 18.8, -47.1,
	5.4, 45.4, 38.1,
	23.7, -33.4, -24.1,
	-37.1, 47.1, 18.5,
	44.7, 18.9, 8.5,
	28.2, 23.0, 25.9,
	-14.0, 22.9, -47.5,
	};
	
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(starsPositions), starsPositions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0); //position
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	return VAO;
}



//////////////////////////////////////////
////        COORDINATE SYSTEMS         ///
//////////////////////////////////////////
glm::mat4 translateModel(void) {
	glm::mat4 transMat = glm::translate(glm::mat4(1.0f), glm::vec3(transX, transY, transZ));
	return transMat;
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
	float zFar = 500.0f;
	glm::mat4 projectionMat = glm::perspective(camera.Fov, aspectRatio, zNear, zFar);
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

	cameraMovement[0] = keys[GLFW_KEY_W];
	cameraMovement[1] = keys[GLFW_KEY_S];
	cameraMovement[2] = keys[GLFW_KEY_A];
	cameraMovement[3] = keys[GLFW_KEY_D];
	cameraMovement[4] = keys[GLFW_KEY_SPACE];
	cameraMovement[5] = keys[GLFW_KEY_X];


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