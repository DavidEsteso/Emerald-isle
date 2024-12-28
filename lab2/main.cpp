#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <city.h>

#include <tree.h>
#include <aircraft.h>
#include <entity.h>
#include <sky.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

#include <cmath>
#include <building.h>
#include <ground.h>
#include <bot.h>
#include <memory>

#include <sstream>
#include <iomanip>

#include <tea.h>

static GLFWwindow *window;
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);


float yaw = glm::degrees(atan2(front.z, front.x));


float pitch = glm::degrees(asin(front.y));


float smoothStep(float x) {
	return x * x * (3 - 2 * x); // Función de suavizado
}

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
	window = glfwCreateWindow(1024 , 768  , "Emerald Isle", NULL, NULL);
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

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



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
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);;



	float currentTime = 0.0f;
	InfiniteCity city(window);


	Sky sky;
	sky.initialize(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2000.0f, 2000.0f, 2000.0f), "../lab2/textures/cube_");

	// Camera setup
    eye_center.y = viewDistance * cos(viewPolar);
    eye_center.x = viewDistance * cos(viewAzimuth);
    eye_center.z = viewDistance * sin(viewAzimuth);

	glm::mat4 viewMatrix, projectionMatrix, viewMatrixSky;
    glm::float32 FoV = 45;
	glm::float32 zNear = 0.1f;
	glm::float32 zFar = 6000.0f;
	projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);

	static double lastTime = glfwGetTime();
	float time = 0.0f;
	float fTime = 0.0f;
	unsigned long frames = 0;

	bool hasInteracted = false;



	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);
		lastTime = currentTime;


		viewMatrix = glm::lookAt(eye_center, lookat, up);
		glm::mat4 vp = projectionMatrix * viewMatrix;

		viewMatrixSky = glm::mat4(glm::mat3(viewMatrix)); // Remove translation from the view matrix
		glm::mat4 vpSky = projectionMatrix * viewMatrixSky;
		sky.render(vpSky);

		currentTime = glfwGetTime();

		city.update(eye_center, currentTime);
		city.render(vp, viewMatrix, projectionMatrix, eye_center);

		// Handle aircraft interaction
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !hasInteracted) {
			auto [aircraftPos, aircraftFound] = city.handleAircraftInteraction(eye_center, viewMatrix, projectionMatrix);

			if (aircraftFound) {
				hasInteracted = true;
				eye_center = aircraftPos;
				eye_center.y += 50.0f;
			}
		}

		frames++;
		fTime += deltaTime;
		if (fTime > 2.0f)
		{
			float fps = frames / fTime;

			frames = 0;
			fTime = 0;

			std::stringstream stream;
			stream << std::fixed << std::setprecision(2) << "Frames per second (FPS): " << fps;

			glfwSetWindowTitle(window, stream.str().c_str());

		}
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (!glfwWindowShouldClose(window));

	// Clean up
	city.cleanup();

	sky.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

float mouseSensitivity = 0.4f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	int windowWidth, windowHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	static float lastX = static_cast<float>(windowWidth) / 2.0f;
	static float lastY = static_cast<float>(windowHeight) / 2.0f;
	static bool firstMouse = true;

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	xoffset *= mouseSensitivity;
	yoffset *= mouseSensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	lookat = eye_center + glm::normalize(front);
}


void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    // Camera movement with WASD
    glm::vec3 front = glm::normalize(glm::vec3(lookat.x - eye_center.x, 0.0f, lookat.z - eye_center.z));
    glm::vec3 right = glm::normalize(glm::cross(front, up));

    if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        eye_center += cameraSpeed * front;
        lookat += cameraSpeed * front;
    }
    if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        eye_center -= cameraSpeed * front;
        lookat -= cameraSpeed * front;
    }
    if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        eye_center -= cameraSpeed * right;
        lookat -= cameraSpeed * right;
    }
    if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        eye_center += cameraSpeed * right;
        lookat += cameraSpeed * right;
    }

    if (isCameraMoving)
    {
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
                glm::vec3 viewDir = glm::normalize(lookat - eye_center);

                eye_center.y += cameraSpeed;
                eye_center.y = glm::clamp(eye_center.y, minHeight, maxHeight);

                // Update lookat position maintaining the same view direction
                lookat = eye_center + viewDir;
            }
            if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
                glm::vec3 viewDir = glm::normalize(lookat - eye_center);

                eye_center.y -= cameraSpeed;
                eye_center.y = glm::clamp(eye_center.y, minHeight, maxHeight);

                // Update lookat position maintaining the same view direction
                lookat = eye_center + viewDir;
            }
        }
    }

    // Escape to close window
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}