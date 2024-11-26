
#include <sky.h>
#include <city.h>
#include <model2.h>

#include<model1.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <render/shader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <vector>
#include <iostream>
#define w
#include <math.h>
#include <unordered_map>
#include <cmath>
#include <fastNoiseLite.h>
#include <random>



static GLFWwindow *window;
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// OpenGL camera view parameters
static glm::vec3 eye_center(300.0f, 300.0f, 300.0f); // Position the camera inside the cube
static glm::vec3 lookat(0, 1, 1);
static glm::vec3 up(0, 1, 0);

// View control
float viewPolar = glm::radians(60.0f);
float viewAzimuth = glm::radians(45.0f);
static float viewDistance = 400.0f;

glm::vec3 front = glm::normalize(lookat - eye_center);

float yaw = glm::degrees(atan2(front.z, front.x));


float pitch = glm::degrees(asin(front.y));

const char* skyTexturePaths[6] = {
	"../lab2/textures/cube_front.png",
	"../lab2/textures/cube_back.png",
	"../lab2/textures/cube_left.png",
	"../lab2/textures/cube_right.png",
	"../lab2/textures/cube_up.png",
	"../lab2/textures/cube_down.png"
};



int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW." << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Lab 2", NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Failed to open a GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(window, key_callback);

	// Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0)
	{
		std::cerr << "Failed to initialize OpenGL context." << std::endl;
		return -1;
	}

	// Background
	glClearColor(0.53f, 0.81f, 0.92f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);;


	float width = 16.0f;
	float depth = 16.0f;
	float height = 80.0f;


    InfiniteCity city;
	float currentTime = 0.0f;

	//Tree tree;
	//glm::vec3 tree_position = eye_center + front * 350.0f;
	//tree.initialize("../lab2/models/Tree/Tree.obj", "../lab2/models/Tree", tree_position, glm::vec3(50.0f, 50.0f, 50.0f));

	//Aircraft aircraft;
	//glm::vec3 aircraft_pos = eye_center + front * 350.0f;
	//aircraft.initialize("../lab2/models/air/E45Aircraft_obj.obj", "../lab2/models/air", aircraft_pos, glm::vec3(50.0f, 50.0f, 50.0f));

	Sky sky;
	sky.initialize(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(500.0f, 500.0f, 500.0f), skyTexturePaths);



	// Camera setup
    eye_center.y = viewDistance * cos(viewPolar);
    eye_center.x = viewDistance * cos(viewAzimuth);
    eye_center.z = viewDistance * sin(viewAzimuth);

	glm::mat4 viewMatrix, projectionMatrix, viewMatrixSky;
    glm::float32 FoV = 45;
	glm::float32 zNear = 0.1f;
	glm::float32 zFar = 1000.0f;
	projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);

	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		viewMatrix = glm::lookAt(eye_center, lookat, up);
		glm::mat4 vp = projectionMatrix * viewMatrix;

		viewMatrixSky = glm::mat4(glm::mat3(viewMatrix)); // Remove translation
		glm::mat4 vpSky = projectionMatrix * viewMatrixSky;
		sky.render(vpSky);

		currentTime = glfwGetTime();

		//aircraft.render(vp, eye_center);
		//tree.render(vp, eye_center);
		// Render the building
		city.update(eye_center);
		city.render(vp, viewMatrix, projectionMatrix, eye_center);


		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (!glfwWindowShouldClose(window));
	// Clean up

	// Clean up
	city.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	const float cameraSpeed = 20.0f;
	const float rotationSpeed = 10.0f;
	const float minHeight = 100.0f;
	const float maxHeight = 300.0f;

	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		std::cout << "Reset." << std::endl;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS))
		{
			eye_center.y += cameraSpeed;

			eye_center.y = glm::clamp(eye_center.y, minHeight, maxHeight);

			lookat.y = eye_center.y;
		}

		if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS))
		{
			eye_center.y -= cameraSpeed;

			eye_center.y = glm::clamp(eye_center.y, minHeight, maxHeight);

			lookat.y = eye_center.y;
		}
	}
	else
	{
		if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS))
		{
			glm::vec3 direction = glm::normalize(glm::vec3(lookat.x, 0.0f, lookat.z) - glm::vec3(eye_center.x, 0.0f, eye_center.z));
			eye_center += cameraSpeed * direction;

			eye_center.y = glm::clamp(eye_center.y, minHeight, maxHeight);

			lookat = eye_center + glm::vec3(direction.x, 0.0f, direction.z);
		}

		if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS))
		{
			glm::vec3 direction = glm::normalize(glm::vec3(lookat.x, 0.0f, lookat.z) - glm::vec3(eye_center.x, 0.0f, eye_center.z));
			eye_center -= cameraSpeed * direction;

			eye_center.y = glm::clamp(eye_center.y, minHeight, maxHeight);

			lookat = eye_center + glm::vec3(direction.x, 0.0f, direction.z);
		}
	}


	if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		glm::vec3 right = glm::normalize(glm::cross(lookat - eye_center, up));
		eye_center -= cameraSpeed * right;
		lookat -= cameraSpeed * right;
	}

	if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		glm::vec3 right = glm::normalize(glm::cross(lookat - eye_center, up));
		eye_center += cameraSpeed * right;
		lookat += cameraSpeed * right;
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		pitch += rotationSpeed;
		if (pitch > 89.0f)
			pitch = 89.0f;
	}

	if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		pitch -= rotationSpeed;
		if (pitch < -89.0f)
			pitch = -89.0f;
	}

	if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		yaw -= rotationSpeed;
	}

	if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		yaw += rotationSpeed;
	}

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	lookat = eye_center + glm::normalize(front);
}
