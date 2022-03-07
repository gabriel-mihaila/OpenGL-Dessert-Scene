
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Window.h"
#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>



// window
gps::Window myWindow;

const unsigned int SHADOW_WIDTH = 5096;
const unsigned int SHADOW_HEIGHT = 5096;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;



gps::Camera myCamera(
	glm::vec3(-1.5f, 0.0f, 1.75f),
	glm::vec3(-1.5f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));


bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

gps::Model3D nanosuit;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D desertScene;
gps::Model3D barreira;
gps::Model3D humvee;
gps::Model3D front_wheels;
gps::Model3D back_wheels;
gps::Model3D helicopter_static;
gps::Model3D helicopter_dinamic1;
gps::Model3D helicopter_dinamic2;
gps::Model3D fish;

gps::Shader myCustomShader;
gps::Shader depthMapShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader skyboxShader;

//SKYBOX
gps::SkyBox mySkyBox;
std::vector<const GLchar*> faces;

unsigned int SCR_WIDTH = 1024;
unsigned int SCR_HEIGHT = 768;

//cameraMoves
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float cameraSpeed = 0.1f;
bool firstMouse = true;
float zoom = 45.0f;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

//functionalitati
int bool_spotlight = 0;
GLint spotlightLoc;
int bool_cameraPresentation = 0;
int bool_fog = 0;
GLint fogLoc;
GLfloat front_wheel_rotate_angle;
GLfloat wheel_speed;
GLfloat frontWheelPos = 22.1938f;
GLfloat back_WheelPos = 22.7408f;
GLfloat lastFrontWheelPos = 22.1938f;
GLfloat humveePos = 0.0f;
GLfloat fish_rotate_angle;
GLfloat first_propeller_angle;
GLfloat last_propeller_angle;
GLfloat helicopterYPos = 0.0f;
GLfloat helicopterPosition = 0.080191f;
GLfloat helicopterPad = -0.1575f;
bool start = false;

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
	glViewport(0, 0, width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;
	
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS) {
		firstMouse = true;
	}
	else
	{
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.1f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw -= xoffset;
		pitch -= yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		myCamera.rotate(pitch, yaw);
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	zoom -= (float)yoffset;
	if (zoom < 1.0f)
		zoom = 1.0f;
	if (zoom > 45.0f)
		zoom = 45.0f;

	projection = glm::perspective(glm::radians(zoom), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 50.0f);
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 0.1f;
		//if(lightAngle <)
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 0.1f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_1]) {
		bool_spotlight = 1;
		myCustomShader.useShaderProgram();
		spotlightLoc = glGetUniformLocation(myCustomShader.shaderProgram, "bool_spotlight");
		glUniform1i(spotlightLoc, bool_spotlight);
	}

	if (pressedKeys[GLFW_KEY_2]) {
		bool_spotlight = 0;
		myCustomShader.useShaderProgram();
		spotlightLoc = glGetUniformLocation(myCustomShader.shaderProgram, "bool_spotlight");
		glUniform1i(spotlightLoc, bool_spotlight);
	}

	if (pressedKeys[GLFW_KEY_C]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_V]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_B]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	if (pressedKeys[GLFW_KEY_3]) {
		bool_cameraPresentation = 1;
		glfwSetTime(0.0f);
	}

	if (pressedKeys[GLFW_KEY_4]) {
		bool_cameraPresentation = 0;
		myCamera.setCameraPosition (glm::vec3(-1.5f, 0.0f, 1.75f));
		myCamera.setCameraTarget (glm::vec3(-1.5f, 0.0f, 0.0f));
		myCamera.setCameraUp ( glm::vec3(0.0f, 1.0f, 0.0f));
		myCamera.setCameraFrontDirection(glm::normalize(myCamera.getCameraTarget() - myCamera.getCameraPosition()));
		myCamera.setCameraRightDirection(glm::normalize(glm::cross(myCamera.getCameraFrontDirection(), myCamera.getCameraUp())));
	}

	if (pressedKeys[GLFW_KEY_F]) {
		bool_fog = 1;
		myCustomShader.useShaderProgram();
		fogLoc = glGetUniformLocation(myCustomShader.shaderProgram, "bool_fog");
		glUniform1i(fogLoc, bool_fog);
	}

	if (pressedKeys[GLFW_KEY_G])
	{
		bool_fog = 0;
		myCustomShader.useShaderProgram();
		fogLoc = glGetUniformLocation(myCustomShader.shaderProgram, "bool_fog");
		glUniform1i(fogLoc, bool_fog);
	}

	if (pressedKeys[GLFW_KEY_UP])
	{
		//mergi = 1;
		//glfwSetTime(0.0f);
		front_wheel_rotate_angle += 1.5f;
		wheel_speed -= 0.002f;
		humveePos -= 0.002f;

	}

	if (pressedKeys[GLFW_KEY_DOWN]) {
		//mergi = 0;
		front_wheel_rotate_angle -= 1.5f;
		humveePos += 0.002f;
		wheel_speed += 0.002f;
	}

	if (pressedKeys[GLFW_KEY_I]) {
		fish_rotate_angle += 1.5f;
	}

	if (pressedKeys[GLFW_KEY_O]) {
		fish_rotate_angle -= 1.5f;
	}

	if (pressedKeys[GLFW_KEY_5]) {
		first_propeller_angle += 10.0f;
		last_propeller_angle -= 14.0f;
		helicopterYPos += 0.003f;
	}

	if (pressedKeys[GLFW_KEY_6]) {
		if (start) {
			first_propeller_angle -= 10.0f;
			last_propeller_angle += 14.0f;
			helicopterYPos -= 0.003f;
		}
		
	}

}

void initOpenGLWindow()
{
	myWindow.Create(1024, 768, "Future Desert Scene 2.0");
	//glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
	glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
	glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
	glfwSetScrollCallback(myWindow.getWindow(), scroll_callback);

}

void initOpenGLState()
{
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	//glEnable(GL_CULL_FACE); // cull face
	//glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	nanosuit.LoadModel("objects/nanosuit/nanosuit.obj");
	ground.LoadModel("objects/ground/ground.obj");
	lightCube.LoadModel("objects/cube/cube.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
	desertScene.LoadModel("objects/scena_v2/scena_v2.obj");
	barreira.LoadModel("objects/barreira/barreira.obj");
	humvee.LoadModel("objects/humvee/humvee.obj");
	front_wheels.LoadModel("objects/front_wheels/front_wheels.obj");
	back_wheels.LoadModel("objects/back_wheels/back_wheels.obj");
	helicopter_static.LoadModel("objects/helicopter_static/helicopter_static.obj");
	helicopter_dinamic1.LoadModel("objects/helicopter_dinamic1/helicopter_dinamic1.obj");
	helicopter_dinamic2.LoadModel("objects/helicopter_dinamic2/helicopter_dinamic2.obj");
	fish.LoadModel("objects/fish/fish.obj");
}

void initSkyBox() {
	faces.push_back("textures/skybox/right.tga");
	faces.push_back("textures/skybox/left.tga");
	faces.push_back("textures/skybox/top.tga");
	faces.push_back("textures/skybox/bottom.tga");
	faces.push_back("textures/skybox/back.tga");
	faces.push_back("textures/skybox/front.tga");
	mySkyBox.Load(faces);
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
	depthMapShader.useShaderProgram();
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(1.0f, 0.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	spotlightLoc = glGetUniformLocation(myCustomShader.shaderProgram, "bool_spotlight");
	glUniform1i(spotlightLoc, bool_spotlight);

	fogLoc = glGetUniformLocation(myCustomShader.shaderProgram, "bool_fog");
	glUniform1i(fogLoc, bool_fog);

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	glGenFramebuffers(1, &shadowMapFBO);
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
 


glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir * 18.4f, glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//const GLfloat near_plane = 1.0f, far_plane = 100.0f;
	glm::mat4 lightProjection = glm::ortho(-32.8f, 31.0f, -34.6f, 32.6f, 0.03f, 45.6f);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}

void renderScene(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	//do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	desertScene.Draw(shader);
}

void renderBareira(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();

	
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	//do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	barreira.Draw(shader);
}

void renderHumvee(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();


	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(humveePos, 0.0f, 0.0f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	//do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	humvee.Draw(shader);
}

void renderFront_wheels(gps::Shader shader, bool depthPass)
{
	
	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(wheel_speed, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(frontWheelPos, -0.313932f, 12.8455f));
	model = glm::rotate(model, glm::radians(front_wheel_rotate_angle), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-frontWheelPos, 0.313932f, -12.8455f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	

	//do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	front_wheels.Draw(shader);
}

void renderBack_wheels(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(wheel_speed, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(back_WheelPos, -0.313932f, 12.8455f));
	model = glm::rotate(model, glm::radians(front_wheel_rotate_angle), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-back_WheelPos, 0.313932f, -12.8455f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	//do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	back_wheels.Draw(shader);
}

void renderHelicopter_static(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	if (helicopterPosition + helicopterYPos - helicopterPad > 0.23769f) {
		model = glm::translate(model, glm::vec3(0.0f, helicopterYPos, 0.0f));
	}
	
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	//do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	helicopter_static.Draw(shader);
}

void renderHelicopter_dinamic1(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	if (helicopterPosition + helicopterYPos - helicopterPad > 0.23769f) {
		model = glm::translate(model, glm::vec3(0.0f, helicopterYPos, 0.0f));
		model = glm::translate(model, glm::vec3(22.3061f, 0.3071f, 9.28849f));
		model = glm::rotate(model, glm::radians(first_propeller_angle), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(-22.3061f, -0.3071f, -9.28849f));
	}
	
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	//do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	helicopter_dinamic1.Draw(shader);
}

void renderHelicopter_dinamic2(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	if (helicopterPosition + helicopterYPos - helicopterPad > 0.23769f) {
		start = true;
		model = glm::translate(model, glm::vec3(0.0f, helicopterYPos, 0.0f));
		model = glm::translate(model, glm::vec3(22.306f, 0.535461f, 9.32201f));
		model = glm::rotate(model, glm::radians(first_propeller_angle), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(-22.306f, -0.535461f, -9.32201f));
	}
	else{
		start = false;
	}
	
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	//do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	helicopter_dinamic2.Draw(shader);
}

void renderFish(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-19.0532f, -3.09549f, 12.2675f));
	model = glm::rotate(model, glm::radians(fish_rotate_angle), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(19.0532f, 3.09549f, -12.2675f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	//do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	fish.Draw(shader);
}

void renderSkyBox(gps::Shader shader) {
	shader.useShaderProgram();
	view = myCamera.getViewMatrix();

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	mySkyBox.Draw(shader, view, projection);

}

void renderScene() {

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	renderScene(depthMapShader, 1);
	renderBareira(depthMapShader, 1);
	renderHumvee(depthMapShader, 1);
	renderFront_wheels(depthMapShader, 1);
	renderBack_wheels(depthMapShader, 1);
	renderHelicopter_static(depthMapShader, 1);
	renderHelicopter_dinamic1(depthMapShader, 1);
	renderHelicopter_dinamic2(depthMapShader, 1);
	renderFish(depthMapShader, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, (float)myWindow.getWindowDimensions().width , (float)myWindow.getWindowDimensions().height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {


		// final scene rendering pass (with shadows)

		glViewport(0, 0, (float)myWindow.getWindowDimensions().width, (float)myWindow.getWindowDimensions().height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		if (bool_cameraPresentation == 1)
		{
			myCamera.cameraPresentation();
		}

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		renderScene(myCustomShader, false);
		renderBareira(myCustomShader, false);
		renderHumvee(myCustomShader, false);
		renderFront_wheels(myCustomShader, false);
		renderBack_wheels(myCustomShader, false);
		renderHelicopter_static(myCustomShader, false);
		renderHelicopter_dinamic1(myCustomShader, false);
		renderHelicopter_dinamic2(myCustomShader, false);
		renderFish(myCustomShader, false);
		//draw a white cube around the light

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = lightRotation;
		model = glm::translate(model, 1.0f * lightDir);
		model = glm::scale(model, glm::vec3(0.0001f, 0.0001f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		lightCube.Draw(lightShader);

		
	}

	renderSkyBox(skyboxShader);
	
}

void cleanup() {
	myWindow.Delete();
}

int main(int argc, const char* argv[]) {

	try {
		initOpenGLWindow();
	}
	catch (const std::exception & e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	setWindowCallbacks();
	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initSkyBox();
	
	
	
	glCheckError();

	while (!glfwWindowShouldClose(myWindow.getWindow())) {
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());
		glCheckError();
	}

	cleanup();

	return EXIT_SUCCESS;
}
