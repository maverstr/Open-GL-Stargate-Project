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
#include "ParticleGenerator.h"
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
bool showFPSBool = false;

//VAO Creations
GLuint createAxisVAO(void);
GLuint createCubeMapVAO(void);
GLuint createCubeMapTexture(void);
GLuint createStarsVAO(int* starsCount);
void createAsteroidVAO(int asteroidAmount, Model asteroidModel, glm::vec3 planetPos);
GLuint createFramebufferQuadVAO(void);

//draw calls
void drawSkybox();
void drawAxis();
void drawStargate();
void drawSun();
void drawPlanet();
void drawAsteroids();
void drawParticles();
void drawMissile();
void drawWeirdCubes();
void drawJumper();
void drawStars();
void drawLightBulb(glm::vec4 position);
void drawJumperOutlining();

void drawPlanetShadow();
void drawAsteroidsShadow();
void drawMissileShadow();
void drawJumperShadow();
void drawWeirdCubesShadow();
void drawStargateShadow();
void drawLightBulbShadow(glm::vec4 position);

//movements
void movementHandler();

//framebuffer pov
bool followCameraPOV = true;

//Coordinate systems
glm::mat4 moveModel(Jumper jumper, bool outlining);
glm::mat4 createModelMissile(Jumper jumper);
void captureMissileSettings(Jumper jumper);
glm::mat4 createProjectionMatrix(void);
glm::mat4 createViewMatrix1(void);
glm::mat4 createViewMatrix2(void);
glm::mat4 createModelMatrix(void);


 // WINDOW PARAMETERS 
const int windowWidth = 1690;
const int windowHeight = 1050;
const char* windowTitle = "Stargate Project";
bool MSAA = true; 

//movements jumper
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

//camera1
Camera camera1(glm::vec3(22.0f, 16.0f, -2.0f));
//Camera2 (second POV, following jumper)
Camera camera2(glm::vec3(22.0f, 16.0f, -2.0f));
int grayscale = 0;
int kernel = 0;
int blur = 0;
int sharpen = 0;
int edgeDetection = 0;

//lights
vector<LightSource*> lightArray; //array of pointers to all light sources. REMEMBER TO DELETE POINTERS AS I DELETE THE OBJECTS
int lightCounter = 0;

//jumper
Jumper jumper1;
glm::vec3 flashlightJumperOffset = glm::vec3(0.0f, -1.27f, 5.5f);
glm::vec3 jumperFirstPersonOffset = glm::vec3(0.0f, 0.0f, 5.5f);
bool jumperOutlining = true;
float timeOfExplosion = 0.0f;
float explosionDistance = 0;
float maxExplosionDistance = -1;
bool isExploded = false;

//stargate
glm::vec3 stargatePos = glm::vec3(-15.0f, -15.0f, -5.0f);
float stargateAngle = 0.0f;
glm::vec3 distStargate = glm::vec3(0.0f);
float distanceStargate = 0.0f;
float angleStargateFOV = 0.0f;

//missile
glm::vec3 missilePosition = glm::vec3(0.0f);
glm::vec3 missileDirection = glm::vec3(0.0f);
glm::vec3 missileBasePosition = glm::vec3(0.0f);
glm::mat4 missileOrientation = glm::mat4(0.0f);
bool missileLaunched = false;
bool boolCaptureMissileSettings = false;
float missileTimeCounter = 0.0f;

//particles
float dt = 0.004f; //life delta: 2000 particles, 8 spawns per frame, 1 of life at start => 0.004 of delta per frame

//stars
int starsCount = 0;

//planet
glm::vec3 planetPos = glm::vec3(-400.0f, -150.0f, 120.0f);
float planetRotation = 0.0f;
int planetReflection = 0;
float planetRefractionRatio = 0.0f;

//sun
glm::vec3 sunPos = glm::vec3(-200.0f, -150.0f, -100.0f);
glm::vec3 distSun = glm::vec3(0.0f);
float distanceSun = 0.0f;
float angleSunFOV = 0.0f;

//asteroids
unsigned int asteroidAmount = 10000; //best looking results are 10000 but cpu expensive even with instancing

//weird cube
int weirdCubeNormalMapping = 1;
float weirdCubeAngle = 0.0f;

//Shaders
Shader axisShader, skyboxShader, stargateShader, waterPlaneStargateShader, jumperShader, modelOutliningShader, planetShader,
sunShader, asteroidShader, starsShader, missileShader, lightShader, particleShader, lightBulbCenterShader, lightBulbGlassShader,
weirdCubeShader, framebufferShader, shadowShader;

//Textures
GLuint skyboxTexture, jumperReflectionMap, sunTexture, weirdCubeNormalMapTexture;

//VAO
GLuint AxisVAO, skyboxVAO, starsVAO, quadVAO;

//Models
Model StargateModel, waterPlaneStargateModel, JumperModel, PlanetModel, AsteroidModel, SunModel, missileModel, lightBulbCenterModel,
lightBulbGlassModel, weirdCubeModel;

//particles
ParticleGenerator* Particles;

//Coordinate system matrix initialization
glm::mat4 modelMatrix = glm::mat4(0);
glm::mat4 viewMatrix = glm::mat4(0);
glm::mat4 projectionMatrix = glm::mat4(0);


//shadows
float near_plane = 1.0f;
float far_plane = 600.0f;
unsigned int depthCubemap;
bool shadowBool = true;


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
	glfwWindowHint(GLFW_SAMPLES, 4); //MSAA (hint GLFW for a multisample buffer with 4 samples instead of normal color buffer)
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

	MSAA = true;
	glEnable(GL_MULTISAMPLE); //activates MSAA

	glEnable(GL_PROGRAM_POINT_SIZE); //allows to modify the point size (used in stars)

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //allows some transparency
	glEnable(GL_BLEND);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); //do nothing if stencil and depth tests fail (keep values) but replace with 1's if succeed

	//Shaders
	axisShader = Shader("Shaders/axis.vert", "Shaders/axis.frag");
	axisShader.compile();

	skyboxShader = Shader("Shaders/skybox.vert", "Shaders/skybox.frag");
	skyboxShader.compile();

	stargateShader = Shader("Shaders/model.vert", "Shaders/model.frag", "Shaders/model.geom");
	stargateShader.compile();

	waterPlaneStargateShader = Shader("Shaders/waterPlaneStargate.vert", "Shaders/waterPlaneStargate.frag");
	waterPlaneStargateShader.compile();

	jumperShader = Shader("Shaders/model.vert", "Shaders/model.frag", "Shaders/model.geom");
	jumperShader.compile();

	modelOutliningShader = Shader("Shaders/modelOutlining.vert", "Shaders/modelOutlining.frag");
	modelOutliningShader.compile();

	planetShader = Shader("Shaders/planet.vert", "Shaders/planet.frag"); 
	planetShader.compile();

	sunShader = Shader("Shaders/sun.vert", "Shaders/sun.frag");
	sunShader.compile();

	asteroidShader = Shader("Shaders/asteroid.vert", "Shaders/asteroid.frag");
	asteroidShader.compile();

	starsShader = Shader("Shaders/stars.vert", "Shaders/stars.frag");
	starsShader.compile();
	
	missileShader = Shader("Shaders/missile.vert", "Shaders/missile.frag");
	missileShader.compile();

	lightShader = Shader("Shaders/lightSource.vert", "Shaders/lightSource.frag");
	lightShader.compile();

	particleShader = Shader("Shaders/particle.vert", "Shaders/particle.frag");
	particleShader.compile();

	lightBulbCenterShader = Shader("Shaders/lightBulbCenter.vert", "Shaders/lightBulbCenter.frag");
	lightBulbCenterShader.compile();

	lightBulbGlassShader = Shader("Shaders/lightBulbGlass.vert", "Shaders/lightBulbGlass.frag");
	lightBulbGlassShader.compile();

	weirdCubeShader = Shader("Shaders/weirdCube.vert", "Shaders/weirdCube.frag");
	weirdCubeShader.compile();

	framebufferShader = Shader("Shaders/framebuffer.vert", "Shaders/framebuffer.frag");
	framebufferShader.compile();

	shadowShader = Shader("Shaders/shadowShader.vert", "Shaders/shadowShader.frag", "Shaders/shadowShader.geom");
	shadowShader.compile();


	//Textures
	skyboxTexture = createCubeMapTexture();
	jumperReflectionMap = loadTexture("Models/reflectionMapJumper.png");
	sunTexture = loadTexture("Models/2k_sun.jpg");
	weirdCubeNormalMapTexture = loadTexture("Models/weirdCubeNormalMap.png");

	//VAO instanciation
	AxisVAO = createAxisVAO();
	skyboxVAO = createCubeMapVAO();
	starsVAO = createStarsVAO(&starsCount);
	quadVAO = createFramebufferQuadVAO();

	//Models
	StargateModel = Model("Models/Stargate.obj"); //Stargate
	waterPlaneStargateModel = Model("Models/waterPlaneStargate.obj");

	JumperModel = Model("Models/Jumper.obj");
	jumper1.setModel(&JumperModel);

	PlanetModel = Model("Models/planet.obj");

	AsteroidModel = Model("Models/rock.obj");
	createAsteroidVAO(asteroidAmount, AsteroidModel, planetPos); //no return value as there is one VAO per asteroid...

	SunModel = Model("Models/Sun.obj");

	missileModel = Model("Models/missile.obj");

	lightBulbCenterModel = Model("Models/lightBulbCenter.obj");
	lightBulbGlassModel = Model("Models/lightBulbGlass.obj");

	weirdCubeModel = Model("Models/weirdCube.obj");


	//particles
	Particles = new ParticleGenerator(particleShader, 2000);


	//lights
	//args = pointer, pos, [color] OR [ambient, diffuse, specular,], [constant, linear, quadratic attenuation,] [spotlight direction, inner angle, outer angle,] size, VAO from other lights (0 if none already created)
	LightSource rotatingLight = LightSource(&lightCounter, POINTLIGHT, glm::vec3(5.0f, 2.0f, 10.0f), glm::vec3(0.9f, 0.95f, 0.4f), 1.0f, 0.0008f, 0.00002f, 0.5f, 0);
	lightArray.push_back(&rotatingLight);
	GLuint lightVAO = rotatingLight.getVAO();
	
	LightSource flashLight = LightSource(&lightCounter, SPOTLIGHT, glm::vec3(5.0f, 2.0f, 10.0f), glm::vec3(0.9f, 0.95f, 0.4f), glm::vec3(0.9f, 0.95f, 0.4f), glm::vec3(0.9f, 0.95f, 0.4f), 1.0f, 0.0032f, 0.0008f, camera.Front, 4.5f, 6.5f, 0.5f, 0);
	lightArray.push_back(&flashLight);
	LightSource jumperFlashLight = LightSource(&lightCounter, SPOTLIGHT, jumper1.Position + flashlightJumperOffset, glm::vec3(0.9f, 0.95f, 0.4f), glm::vec3(0.9f, 0.95f, 0.4f), glm::vec3(0.9f, 0.95f, 0.4f), 1.0f, 0.0032f, 0.0008f, glm::vec3(0.0f,0.0f,1.0f), 8.5f, 12.5f, 0.2f, 0);
	lightArray.push_back(&jumperFlashLight);
	
	//note: sunlight should always at index 4 in the list (used in shadows for ease)
	LightSource sunLight = LightSource(&lightCounter, POINTLIGHT, sunPos, glm::vec3(1.0f, 0.6f, 0.2f)*1.0f, 1.0f, 0.00080f, 0.0000070f, 1.0f, lightVAO);
	lightArray.push_back(&sunLight);

	LightSource waterStargateLight = LightSource(&lightCounter, POINTLIGHT, stargatePos, glm::vec3(0.3f, 0.5f, 1.0f) *3.0f, 1.0f, 0.0016f, 0.008f, 0.5f, lightVAO);
	lightArray.push_back(&waterStargateLight);

	//to debug a light, comment the push back for the others and set lightCounter = 1;

	
	//shadows: configure framebuffer object
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	//generating depth cubemap
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// attach depth texture as the framebuffer depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE); //no color so explicitely sets the color buffer to none
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	

	//framebuffer configuration for second POV
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// create a color attachment texture
	unsigned int textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



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

		if (showFPSBool)
			showFPS();


		//Handle movements
		movementHandler();


		//the whole render process in 5 steps:
		//1)Render the scene from the sun point of view and create a depth cubemap
		//2)render the scene in a framebuffer that writes color to a texture.(using camera2)
		//3)the scene is rendered again (camera1) but this time in the default framebuffer and fills the entire screen.
		//4)The first scene is then rendered on a quad set up in the upper right corner in NDC.
		//5)An additional pass is required to draw the outline of the jumper by using the stencil buffer for the default framebuffer.

		if (shadowBool) {
			//point shadow mapping: generate the projection matrix from a light and 6 view matrix for each face of the cubemap
			//90degree FOV for each face of the cubemap
			glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
			std::vector<glm::mat4> shadowTransforms;
			shadowTransforms.push_back(shadowProj * glm::lookAt(glm::vec3(sunLight.Position), glm::vec3(sunLight.Position) + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowTransforms.push_back(shadowProj * glm::lookAt(glm::vec3(sunLight.Position), glm::vec3(sunLight.Position) + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowTransforms.push_back(shadowProj * glm::lookAt(glm::vec3(sunLight.Position), glm::vec3(sunLight.Position) + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
			shadowTransforms.push_back(shadowProj * glm::lookAt(glm::vec3(sunLight.Position), glm::vec3(sunLight.Position) + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
			shadowTransforms.push_back(shadowProj * glm::lookAt(glm::vec3(sunLight.Position), glm::vec3(sunLight.Position) + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
			shadowTransforms.push_back(shadowProj * glm::lookAt(glm::vec3(sunLight.Position), glm::vec3(sunLight.Position) + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));


			//render the scene to the depth cubemap
			glEnable(GL_DEPTH_TEST);
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			shadowShader.use();
			for (unsigned int i = 0; i < 6; ++i) {
				shadowShader.setMatrix4(("shadowMatrices[" + std::to_string(i) + "]").c_str(), shadowTransforms[i]);
			}
			shadowShader.setFloat("far_plane", far_plane);
			shadowShader.setVector3f("lightPos", sunLight.Position);
			//render the scene to get the depth cubemap
			drawPlanetShadow();
			drawAsteroidsShadow();
			drawMissileShadow();
			drawJumperShadow();
			drawWeirdCubesShadow();
			drawStargateShadow();
			drawLightBulbShadow(rotatingLight.Position);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		else { //if no shadows, just clean up the texture (otherwise the last shadows stay rendered)
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}


		

		glViewport(0, 0, windowWidth, windowHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		camera.copyThisCamera(camera2); //set the camera with the attributes of cam2
		//Calculate coordinate systems every frame
		viewMatrix = createViewMatrix2();
		projectionMatrix = createProjectionMatrix();
		drawSkybox();
		drawAxis();
		drawStargate();
		drawSun();
		drawPlanet();
		drawAsteroids();
		drawParticles();//particles update, must be rendered before missile and jumper otherwise will be rendered above
		drawMissile();
		drawWeirdCubes();
		drawJumper();
		drawStars();
		drawLightBulb(rotatingLight.Position);		//draw light Bulb (Center and glass with blending)
		if (jumperOutlining) {
			drawJumperOutlining();
		}


		glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default framebuffer
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glStencilMask(0x00); //makes sure we don't update stencil buffer by mistake
		glEnable(GL_DEPTH_TEST);
		camera.copyThisCamera(camera1);//set the camera with the attributes of cam1
		//Calculate coordinate systems every frame
		viewMatrix = createViewMatrix1();
		projectionMatrix = createProjectionMatrix();
		drawSkybox();
		drawAxis();
		drawStargate();
		drawSun();
		drawPlanet();
		drawAsteroids();
		drawParticles();//particles update, must be rendered before missile and jumper otherwise will be rendered above
		drawMissile();
		drawWeirdCubes();
		drawJumper();
		drawStars();
		drawLightBulb(rotatingLight.Position);//draw light Bulb (Center and glass with blending)

		//2nd render pass: outlining of the jumper
		if (jumperOutlining) {
			drawJumperOutlining();
		}

		if (followCameraPOV) { //toggles the second pov
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		framebufferShader.use();
		framebufferShader.setInteger("grayscale", grayscale);
		framebufferShader.setInteger("kernel", kernel);
		framebufferShader.setInteger("sharpen", sharpen);
		framebufferShader.setInteger("blur", blur);
		framebufferShader.setInteger("edgeDetection", edgeDetection);
		glBindVertexArray(quadVAO);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		
		//lights position update (and debug drawing)
		lightShader.use();
		flashLight.updateFlashLightDirection(camera.Front);
		flashLight.updatePosition(camera.Position);
		jumperFlashLight.updateFlashLightDirection(jumper1.Front);
		jumperFlashLight.updatePosition(jumper1.Position + glm::vec3(jumper1.Right * flashlightJumperOffset.x) + glm::vec3(jumper1.Up * flashlightJumperOffset.y) + glm::vec3(jumper1.Front * flashlightJumperOffset.z));
		//jumperFlashLight.draw(lightShader, glm::mat4(1.0f), viewMatrix, projectionMatrix, camera.Position);

		rotatingLight.updatePosition(glm::vec3(sin(glfwGetTime() * 0.6f) * 10.0f, cos(glfwGetTime() * 0.3f) * 7.0f, sin(glfwGetTime()) * 8.0f));
		//rotatingLight.draw(lightShader, glm::mat4(1.0f), viewMatrix, projectionMatrix, camera.Position);
		//sunLight.draw(lightShader, glm::mat4(1.0f), viewMatrix, projectionMatrix, camera.Position);
		waterStargateLight.updatePosition(stargatePos);
		//waterStargateLight.draw(lightShader, glm::mat4(1.0f), viewMatrix, projectionMatrix, camera.Position);

		// Flip Buffers and Draw
		glfwSwapBuffers(mWindow);
		glfwPollEvents();
	}
	glfwTerminate();
	return EXIT_SUCCESS;
}


//////////////////////////////////////////
////               VAO                 ///
//////////////////////////////////////////
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


//CUBEMAP SKYBOX 
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

GLuint createStarsVAO(int* starsCount) {
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

GLuint createFramebufferQuadVAO() {
	float quadVertices[] = { // vertex attributes for a quad that fills a part of the screen in Normalized Device Coordinates.
	// positions   // texCoords
	0.5f,  1.0f,  0.0f, 1.0f,
	0.5f, 0.5f,  0.0f, 0.0f,
	 1.0f, 0.5f,  1.0f, 0.0f,

	0.5f,  1.0f,  0.0f, 1.0f,
	 1.0f, 0.5f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
	};

	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	return quadVAO;
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
glm::mat4 createModelMissile(Jumper jumper) {
	glm::mat4 modelMat = glm::mat4(1.0f);
	if (missileLaunched) {
		float time = glfwGetTime();
		modelMat = missileOrientation;
		glm::vec3 flightDisplacement = missileDirection * (time - missileTimeCounter) * 40.0f;
		missilePosition = missileBasePosition + flightDisplacement;
		modelMat[3] = glm::vec4(missilePosition, 1.0f);
	}
	else {
	missileOrientation = jumper.rotMatTotal;
	missileDirection = jumper.Front;
	modelMat = missileOrientation;
	missilePosition = jumper.Position - (glm::vec3(jumper.Up) * 1.3f);
	modelMat[3] = glm::vec4(missilePosition, 1.0);
	}
	return modelMat;
}

void captureMissileSettings(Jumper jumper) { //called when missile launched to set the parameters
	missileDirection = jumper.Front;
	missileOrientation = jumper.rotMatTotal;
	missileBasePosition = glm::vec3(jumper.Position - (glm::vec3(jumper.Up) * 1.3f));
	missileTimeCounter = glfwGetTime();
}

glm::mat4 createViewMatrix1(void) {
	glm::mat4 ViewMat = camera.GetViewMatrix();
	return ViewMat;
}

glm::mat4 createViewMatrix2(void) {
	glm::mat4 ViewMat = glm::lookAt(camera.Position, camera.Position + camera.Front, -glm::vec3(jumper1.Up));
	return ViewMat;
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
	if (keys[GLFW_KEY_Z]) {
		static bool vsync = true;
		if (vsync) {
			showFPSBool = false;
			glfwSwapInterval(1);
		}
		else {
			showFPSBool = true;
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

	//missile Launch
	if (keys[GLFW_KEY_L]) {
		if (!missileLaunched) {
			missileLaunched = true;
			boolCaptureMissileSettings = true;
		}
	}

	//Planet reflection toggle
	if (keys[GLFW_KEY_U]) {
		if (planetReflection == 0) {
			planetReflection = 1;
			planetRefractionRatio = 0.0f; //deactivate refraction as well
		}
		else
			planetReflection = 0;
	}

	//Planet refraction toggle
	if (keys[GLFW_KEY_I]) {
		if (planetRefractionRatio == 0.0f) {
			planetRefractionRatio = 0.85f;
			planetReflection = 0; //deactivate reflection as well
		}
		else
			planetRefractionRatio = 0.0f;
	}

	//sound
	if (keys[GLFW_KEY_P])
		musicBool = true;

	//MSAA
	if (keys[GLFW_KEY_N]) {
		if (MSAA) {
			glDisable(GL_MULTISAMPLE);
			MSAA = false;
		}
		else {
			glEnable(GL_MULTISAMPLE);
			MSAA = true;
		}
	}

	//Weird cube normal mapping
	if (keys[GLFW_KEY_V]) {
		if (weirdCubeNormalMapping == 1)
			weirdCubeNormalMapping = 0;
		else
			weirdCubeNormalMapping = 1;
	}

	//follow Camera POV (framebuffer)
	if (keys[GLFW_KEY_J]) {
		followCameraPOV = !followCameraPOV;
	}

	//Options for post-processing effects on camera2
	if (keys[GLFW_KEY_KP_7]) {
		if (grayscale == 1) {
			grayscale = 0;
			kernel = 0;
		}
		else {
			grayscale = 1;
			kernel = 0;
		}
	}

	if (keys[GLFW_KEY_KP_4]) {
		if (sharpen == 1) {
			sharpen = 0;
			grayscale = 0;
			kernel = 0;
			edgeDetection = 0;
			blur = 0;
		}
		else {
			sharpen = 1;
			kernel = 1;
			blur = 0;
			grayscale = 0;
			edgeDetection = 0;
		}
	}

	if (keys[GLFW_KEY_KP_5]) {
		if (blur == 1) {
			blur = 0;
			grayscale = 0;
			kernel = 0;
			edgeDetection = 0;
			blur = 0;
		}
		else {
			blur = 1;
			kernel = 1;
			sharpen = 0;
			grayscale = 0;
			edgeDetection = 0;
		}
	}

	if (keys[GLFW_KEY_KP_6]) {
		if (edgeDetection == 1) {
			edgeDetection = 0;
			grayscale = 0;
			kernel = 0;
			blur = 0;
			blur = 0;
		}
		else {
			edgeDetection = 1;
			kernel = 1;
			sharpen = 0;
			grayscale = 0;
			blur = 0;
		}
	}

	//Shadow toggle
	if (keys[GLFW_KEY_KP_8]) {
		shadowBool = !shadowBool;
	}

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
	camera1.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double /*xoffset*/, double yoffset)
{
	camera1.ProcessMouseScroll(yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void movementHandler() {
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
	//sets the camera somewhere on the right side of the jumper
	camera2.Position = jumper1.Position + glm::vec3(jumper1.Right) * 3.0f - glm::vec3(jumper1.Front) * 2.0f + glm::vec3(jumper1.Up) * 2.0f; 
	camera2.setInitialLookAt(jumper1.Position + glm::vec3(jumper1.Front) * 200.0f); //makes the camera look far in front
	if (cameraMovement[0])
		camera1.ProcessKeyboard(CAM_FORWARD, deltaTime);
	if (cameraMovement[1])
		camera1.ProcessKeyboard(CAM_BACKWARD, deltaTime);
	if (cameraMovement[2])
		camera1.ProcessKeyboard(CAM_LEFT, deltaTime);
	if (cameraMovement[3])
		camera1.ProcessKeyboard(CAM_RIGHT, deltaTime);
	if (cameraMovement[4])
		camera1.ProcessKeyboard(CAM_UP, deltaTime);
	if (cameraMovement[5])
		camera1.ProcessKeyboard(CAM_DOWN, deltaTime);

	if (SprintActivated) {
		camera1.MovementSpeed = FastCameraMovementSpeed;
	}
	else {
		camera1.MovementSpeed = baseCameraMovementSpeed;
	}
}
//////////////////////////////////////////
////	  	  DRAWING FUNCTIONS        ///
//////////////////////////////////////////
void drawSkybox() {
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
}

void drawAxis() {
	glBindVertexArray(AxisVAO);
	axisShader.use();
	axisShader.setMatrix4("model", glm::mat4(1.0f));
	axisShader.setMatrix4("view", viewMatrix);
	axisShader.setMatrix4("projection", projectionMatrix);
	glDrawElements(GL_LINES, 12, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void drawStargate() {
	glDisable(GL_CULL_FACE); //needs to be turned off here since Blender model with triangles not specifically in the correct direction
	stargateShader.use();
	glActiveTexture(GL_TEXTURE15);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	stargateShader.setInteger("skybox", 15);
	stargateShader.setFloat("material.refractionRatio", 0.0f);
	stargateShader.setInteger("material.reflection", 0);
	stargateShader.setInteger("material.reflectionMap", 0);
	stargateAngle -= 0.016f;
	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, stargatePos);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(stargateAngle), glm::vec3(1.0f, 0.0f, 0.0f));
	stargateShader.setMatrix4("model", modelMatrix);
	stargateShader.setMatrix4("view", viewMatrix);
	stargateShader.setMatrix4("projection", projectionMatrix);
	stargateShader.setVector3f("viewPos", camera.Position);
	stargateShader.setFloat("material.shininess", 32.0f);
	stargateShader.setFloat("explosionDistance", -1);

	stargateShader.setInteger("lightCounter", lightCounter); //Sets the number of lights in the environment

	for (int i = 0; i < lightCounter; i++) { //sends the light info to the object shaders
		(*lightArray[i]).setModelShaderLightParameters(stargateShader, i);
	}
	stargateShader.setFloat("far_plane", far_plane);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	stargateShader.setInteger("depthMap", 10);
	StargateModel.Draw(stargateShader);

	waterPlaneStargateShader.use();
	waterPlaneStargateShader.setMatrix4("model", modelMatrix);
	waterPlaneStargateShader.setMatrix4("view", viewMatrix);
	waterPlaneStargateShader.setMatrix4("projection", projectionMatrix);
	waterPlaneStargateShader.setVector3f("stargatePos", stargatePos);
	waterPlaneStargateShader.setFloat("time", glfwGetTime() / 5);
	distStargate = stargatePos - camera.Position;
	distanceStargate = sqrt(pow(distStargate.x, 2) + pow(distStargate.y, 2) + pow(distStargate.z, 2));
	angleStargateFOV = 2 * tan((1.0f) / distanceStargate);
	waterPlaneStargateShader.setFloat("cameraFov", camera.Fov);
	waterPlaneStargateShader.setFloat("angle", glm::degrees(angleStargateFOV));
	waterPlaneStargateShader.setFloat("far_plane", far_plane);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	waterPlaneStargateShader.setInteger("depthMap", 10);
	waterPlaneStargateModel.Draw(waterPlaneStargateShader);
}

void drawStargateShadow() {
	glDisable(GL_CULL_FACE); //needs to be turned off here since Blender model with triangles not specifically in the correct direction
	shadowShader.use();
	stargateAngle -= 0.016f;
	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, stargatePos);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(stargateAngle), glm::vec3(1.0f, 0.0f, 0.0f));
	shadowShader.setMatrix4("model", modelMatrix);
	StargateModel.Draw(shadowShader);

	waterPlaneStargateModel.Draw(shadowShader);
}

void drawSun() {
	glEnable(GL_CULL_FACE); //we can use face culling from here to save performance
	sunShader.use();
	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, sunPos);
	float scale = 60.0f;
	modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
	sunShader.setMatrix4("model", modelMatrix);
	sunShader.setMatrix4("view", viewMatrix);
	sunShader.setMatrix4("projection", projectionMatrix);
	sunShader.setVector3f("sunPos", sunPos);
	sunShader.setFloat("time", glfwGetTime() / 10); //don't move too fast
	sunShader.setFloat("random", ((sin(glfwGetTime()) + 1.0) / 6.0) + 0.4); //random between 0.40 and 0.73
	distSun = sunPos - camera.Position; //distance between sun center and camera
	distanceSun = sqrt(pow(distSun.x, 2) + pow(distSun.y, 2) + pow(distSun.z, 2));
	angleSunFOV = 2 * tan((1.0f * scale) / distanceSun); //angle of the sun in the viewport = atan(radius (=1) * scale /dist)
	sunShader.setFloat("cameraFov", camera.Fov);
	sunShader.setFloat("angle", glm::degrees(angleSunFOV)); //used for ratio between sun angle in viewport and camera Fov
	SunModel.Draw(sunShader);
}

void drawPlanet() {
	glEnable(GL_CULL_FACE); //we can use face culling from here to save performance
	planetShader.use();
	modelMatrix = glm::mat4(1.0f);
	planetRotation += 0.12f;
	if (planetRotation >= 360.0f) {
		planetRotation = 0.0f;
	}
	modelMatrix = glm::rotate(modelMatrix, glm::radians(planetRotation), glm::vec3(0.1f, 1.0f, 0.2f));
	modelMatrix[3] = glm::vec4(planetPos, 1.0f);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(8.0f, 8.0f, 8.0f));
	planetShader.setMatrix4("model", modelMatrix);
	planetShader.setMatrix4("view", viewMatrix);
	planetShader.setMatrix4("projection", projectionMatrix);
	planetShader.setVector3f("viewPos", camera.Position);
	planetShader.setFloat("material.shininess", 16.0f);
	glActiveTexture(GL_TEXTURE15);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	planetShader.setInteger("skybox", 15);
	planetShader.setFloat("material.refractionRatio", planetRefractionRatio);
	planetShader.setInteger("material.reflection", planetReflection);
	planetShader.setInteger("lightCounter", lightCounter); //Sets the number of lights in the environment
	for (int i = 0; i < lightCounter; i++) { //sends the light info to the object shaders
		(*lightArray[i]).setModelShaderLightParameters(planetShader, i);
	}
	planetShader.setFloat("far_plane", far_plane);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	planetShader.setInteger("depthMap", 10);
	PlanetModel.Draw(planetShader);
}

void drawPlanetShadow() {
	glEnable(GL_CULL_FACE); //we can use face culling from here to save performance
	shadowShader.use();
	modelMatrix = glm::mat4(1.0f);
	if (planetRotation >= 360.0f) {
		planetRotation = 0.0f;
	}
	modelMatrix = glm::rotate(modelMatrix, glm::radians(planetRotation), glm::vec3(0.1f, 1.0f, 0.2f));
	modelMatrix[3] = glm::vec4(planetPos, 1.0f);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(8.0f, 8.0f, 8.0f));
	shadowShader.setMatrix4("model", modelMatrix);

	PlanetModel.Draw(shadowShader);
}

void drawAsteroids() {
	glEnable(GL_CULL_FACE); //we can use face culling from here to save performance
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
}

void drawAsteroidsShadow() {
	glEnable(GL_CULL_FACE); //we can use face culling from here to save performance
	shadowShader.use();
	for (unsigned int i = 0; i < AsteroidModel.meshes.size(); i++)
	{
		glBindVertexArray(AsteroidModel.meshes[i].VAO);
		glDrawElementsInstanced(GL_TRIANGLES, AsteroidModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, asteroidAmount);
		glBindVertexArray(0);
	}
}

void drawParticles() {
	Particles->Update(dt, missilePosition, missileDirection, 8, -missileDirection * 4.8f); //rendered on missile position, with an offset to put it at the end, and velocity and its direction
	particleShader.use();
	particleShader.setMatrix4("view", viewMatrix);
	particleShader.setMatrix4("projection", projectionMatrix);
	Particles->Draw();
}

void drawMissile() {
	glDisable(GL_CULL_FACE); //needs to be turned off here since Blender model with triangles not specifically in the correct direction
	glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments from this model should update the stencil buffer
	glStencilMask(0xFF); // enable writing to the stencil buffer
	missileShader.use();
	missileShader.setMatrix4("view", viewMatrix);
	missileShader.setMatrix4("projection", projectionMatrix);
	missileShader.setMatrix4("model", createModelMissile(jumper1));
	missileShader.setVector3f("viewPos", camera.Position);
	missileShader.setFloat("material.shininess", 16.0f);
	missileShader.setInteger("lightCounter", lightCounter); //Sets the number of lights in the environment
	for (int i = 0; i < lightCounter; i++) { //sends the light info to the object shaders
		(*lightArray[i]).setModelShaderLightParameters(planetShader, i);
	}

	if (boolCaptureMissileSettings) { //need to store jumper direction and orientation for the missile to follow its path
		//so i use a flag triggered by glfw keys
		captureMissileSettings(jumper1);
		boolCaptureMissileSettings = false;
	}
	if (missileLaunched && (glfwGetTime() - missileTimeCounter) > 10.0f) {
		//after set time, missile is reset
		missileLaunched = false;
	}
	missileShader.setFloat("far_plane", far_plane);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	missileShader.setInteger("depthMap", 10);
	missileModel.Draw(missileShader);
}

void drawMissileShadow() {
	glDisable(GL_CULL_FACE); //needs to be turned off here since Blender model with triangles not specifically in the correct direction
	shadowShader.use();
	shadowShader.setMatrix4("model", createModelMissile(jumper1));
	missileModel.Draw(shadowShader);
}

void drawWeirdCubes() {
	glEnable(GL_CULL_FACE); //needs to be turned off here since Blender model with triangles not specifically in the correct direction
	weirdCubeShader.use();
	weirdCubeShader.setMatrix4("view", viewMatrix);
	weirdCubeShader.setMatrix4("projection", projectionMatrix);
	weirdCubeShader.setVector3f("viewPos", camera.Position);
	glActiveTexture(GL_TEXTURE12);
	glBindTexture(GL_TEXTURE_2D, weirdCubeNormalMapTexture);
	weirdCubeShader.setInteger("normalMap", 12);
	weirdCubeShader.setFloat("material.shininess", 16.0f);
	weirdCubeShader.setInteger("normalMapping", weirdCubeNormalMapping);
	weirdCubeShader.setInteger("lightCounter", lightCounter); //Sets the number of lights in the environment
	for (int i = 0; i < lightCounter; i++) { //sends the light info to the object shaders
		(*lightArray[i]).setModelShaderLightParameters(planetShader, i);
	}
	glActiveTexture(GL_TEXTURE0);
	weirdCubeShader.setFloat("far_plane", far_plane);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	weirdCubeShader.setInteger("depthMap", 10);

	//6 cubes in rotation around the stargate
	weirdCubeAngle -= 0.25f;
	for (int i = 0; i < 6; i++) {
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, cos((glfwGetTime() * 0.1f) - glm::radians(60.0 * i)) * 20.0f, sin((glfwGetTime() * 0.1f) - glm::radians(60.0 * i)) * 20.0f) + stargatePos);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(weirdCubeAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		weirdCubeShader.setMatrix4("model", modelMatrix);
		weirdCubeModel.Draw(weirdCubeShader);
	}

	//10 cubes in rotation around the planet
	for (int i = 0; i < 10; i++) {
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(cos((glfwGetTime() * 0.1f) - glm::radians(36.0 * i)) * 60.0f, 0.0f , sin((glfwGetTime() * 0.1f) - glm::radians(36.0 * i)) * 60.0f) + planetPos);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(weirdCubeAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		weirdCubeShader.setMatrix4("model", modelMatrix);
		weirdCubeModel.Draw(weirdCubeShader);
	}

	//static one
	modelMatrix = glm::mat4(1.0f);
	modelMatrix[3] = glm::vec4(10.0f, 5.0f, 0.0f, 1.0f);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(weirdCubeAngle), glm::vec3(1.0f, 0.0f, 0.0f));
	weirdCubeShader.setMatrix4("model", modelMatrix);
	weirdCubeModel.Draw(weirdCubeShader);
}

void drawWeirdCubesShadow() {
	glEnable(GL_CULL_FACE); //needs to be turned off here since Blender model with triangles not specifically in the correct direction
	shadowShader.use();
	for (int i = 0; i < 6; i++) {
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, cos((glfwGetTime() * 0.1f) - glm::radians(60.0 * i)) * 20.0f, sin((glfwGetTime() * 0.1f) - glm::radians(60.0 * i)) * 20.0f) + stargatePos);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(weirdCubeAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		shadowShader.setMatrix4("model", modelMatrix);
		weirdCubeModel.Draw(shadowShader);
	}

	//10 cubes in rotation around the planet
	for (int i = 0; i < 10; i++) {
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(cos((glfwGetTime() * 0.1f) - glm::radians(36.0 * i)) * 60.0f, 0.0f, sin((glfwGetTime() * 0.1f) - glm::radians(36.0 * i)) * 60.0f) + planetPos);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(weirdCubeAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		shadowShader.setMatrix4("model", modelMatrix);
		weirdCubeModel.Draw(shadowShader);
	}

	//static one
	modelMatrix = glm::mat4(1.0f);
	modelMatrix[3] = glm::vec4(10.0f, 5.0f, 0.0f, 1.0f);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(weirdCubeAngle), glm::vec3(1.0f, 0.0f, 0.0f));
	shadowShader.setMatrix4("model", modelMatrix);
	weirdCubeModel.Draw(shadowShader);
}

void drawJumper() {
	glDisable(GL_CULL_FACE); //needs to be turned off here since Blender model with triangles not specifically in the correct direction
	glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments from this model should update the stencil buffer
	glStencilMask(0xFF); // enable writing to the stencil buffer
	jumperShader.use();
	glActiveTexture(GL_TEXTURE15);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	jumperShader.setInteger("skybox", 15);
	glActiveTexture(GL_TEXTURE14);
	glBindTexture(GL_TEXTURE_2D, jumperReflectionMap);
	jumperShader.setInteger("material.texture_reflectionMap", 14);
	if (isExploded) {
		explosionDistance = sin(((glfwGetTime() - timeOfExplosion) * 2 - 1) / 3.0f); //center the range and slow down the animation
		if (explosionDistance > 0.99f) {
			maxExplosionDistance = 1;
		}
		jumperShader.setFloat("explosionDistance", max(maxExplosionDistance, explosionDistance));
	}
	else {
		jumperShader.setFloat("explosionDistance", -1);
	}
	jumperShader.setFloat("material.refractionRatio", 0.0f);
	jumperShader.setInteger("material.reflectionMap", 1);
	jumperShader.setInteger("material.reflection", 1);
	jumperShader.setMatrix4("model", moveModel(jumper1, false));
	jumperShader.setMatrix4("view", viewMatrix);
	jumperShader.setMatrix4("projection", projectionMatrix);
	jumperShader.setVector3f("viewPos", camera.Position);
	jumperShader.setFloat("material.shininess", 32.0f);

	jumperShader.setInteger("lightCounter", lightCounter); //Sets the number of lights in the environment
	for (int i = 0; i < lightCounter; i++) { //sends the light info to the object shaders
		(*lightArray[i]).setModelShaderLightParameters(jumperShader, i);
	}
	jumperShader.setFloat("far_plane", far_plane);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	jumperShader.setInteger("depthMap", 10);
	JumperModel.Draw(jumperShader);
}

void drawJumperShadow() {
	glDisable(GL_CULL_FACE); //needs to be turned off here since Blender model with triangles not specifically in the correct direction
	shadowShader.use();
	shadowShader.setMatrix4("model", moveModel(jumper1, false));
	JumperModel.Draw(shadowShader);
}

void drawStars() {
	glEnable(GL_CULL_FACE); //we can use face culling from here to save performance
	glBindVertexArray(starsVAO);
	starsShader.use();
	starsShader.setMatrix4("model", glm::mat4(1.0f));
	starsShader.setMatrix4("view", viewMatrix);
	starsShader.setMatrix4("projection", projectionMatrix);
	glDrawArraysInstanced(GL_POINTS, 0, 1, starsCount); //uses instance drawing for the stars
	glBindVertexArray(0);
}

void drawLightBulb(glm::vec4 position) {
	//draw light bulb center
	glEnable(GL_CULL_FACE);
	lightBulbCenterShader.use();
	lightBulbCenterShader.setMatrix4("view", viewMatrix);
	lightBulbCenterShader.setMatrix4("projection", projectionMatrix);
	modelMatrix = glm::mat4(1.0f);
	modelMatrix[3] = glm::vec4(position);
	lightBulbCenterShader.setMatrix4("model", modelMatrix);
	lightBulbCenterShader.setFloat("far_plane", far_plane);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	lightBulbCenterShader.setInteger("depthMap", 10);
	lightBulbCenterModel.Draw(lightBulbCenterShader);

	//draw light Bulb Glass (blending)
	glEnable(GL_CULL_FACE); //needs to be turned ON here otherwise the blending will mess up with the texture on the other side of the glass.
	lightBulbGlassShader.use();
	lightBulbGlassShader.setMatrix4("view", viewMatrix);
	lightBulbGlassShader.setMatrix4("projection", projectionMatrix);
	lightBulbGlassShader.setMatrix4("model", modelMatrix);
	lightBulbGlassShader.setFloat("far_plane", far_plane);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	lightBulbGlassShader.setInteger("depthMap", 10);
	lightBulbGlassModel.Draw(lightBulbGlassShader);
}

void drawLightBulbShadow(glm::vec4 position) {
	//draw light bulb center
	glEnable(GL_CULL_FACE);
	shadowShader.use();
	modelMatrix = glm::mat4(1.0f);
	modelMatrix[3] = glm::vec4(position);
	shadowShader.setMatrix4("model", modelMatrix);
	lightBulbCenterModel.Draw(shadowShader);

	//draw light Bulb Glass (blending)
	glEnable(GL_CULL_FACE); //needs to be turned ON here otherwise the blending will mess up with the texture on the other side of the glass.
	shadowShader.use();
	lightBulbGlassModel.Draw(shadowShader);
}


void drawJumperOutlining() {
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF); //only the parts not equal to 1 (i.e. the model itself) will be drawn so we don't overwrite the model itself
	glStencilMask(0x00); // disable writing to the stencil buffer
	glDisable(GL_DEPTH_TEST); //outline is always drawn above everything
	modelOutliningShader.use();
	modelOutliningShader.setMatrix4("model", moveModel(jumper1, true));
	modelOutliningShader.setMatrix4("view", viewMatrix);
	modelOutliningShader.setMatrix4("projection", projectionMatrix);
	JumperModel.Draw(modelOutliningShader);
	glStencilMask(0xFF);
	glEnable(GL_DEPTH_TEST);
}


//////////////////////////////////////////
////			 DEBUGGING            ///
//////////////////////////////////////////
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
