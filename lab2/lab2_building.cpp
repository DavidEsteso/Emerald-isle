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

	glfwSetCursorPosCallback(window, mouse_callback);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



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



	float currentTime = 0.0f;
	//city.initializeShadowMapping();
	InfiniteCity city(window);


	Tree tree;
	glm::vec3 tree_position = glm::vec3(eye_center.x + front.x * 50.0f, 100, eye_center.z + front.z * 50.0f);
	//tree.initialize(tree_position, glm::vec3(50.0f, 50.0f, 50.0f));

	//Building b;
	//b.initialize(tree_position, glm::vec3(32.0f, 32.0f, 32.0f), "../lab2/textures/cube_");

	//Ground ground;
	//ground.initialize(tree_position, glm::vec3(500.0f, 500.0f, 500.0f), "../lab2/textures/facade1.jpg");

	//Aircraft aircraft;
	//glm::vec3 aircraft_pos = eye_center + front * 350.0f;
	//aircraft.initialize(aircraft_pos, glm::vec3(50.0f, 50.0f, 50.0f));

	//Sky sky;
	//sky.initialize(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(500.0f, 500.0f, 500.0f), skyTexturePaths);

	MyBot bot;
	bot.initialize(tree_position, glm::vec3(1, 1, 1));


	Tea tea;
	tea.initialize(tree_position, glm::vec3(1, 1, 1));

	Spire spire;
	spire.initialize(tree_position, glm::vec3(1, 1, 1));

	Obelisc obelisc;
	obelisc.initialize(tree_position, glm::vec3(1, 1, 1));



	// Camera setup
    eye_center.y = viewDistance * cos(viewPolar);
    eye_center.x = viewDistance * cos(viewAzimuth);
    eye_center.z = viewDistance * sin(viewAzimuth);

	glm::mat4 viewMatrix, projectionMatrix, viewMatrixSky;
    glm::float32 FoV = 45;
	glm::float32 zNear = 0.1f;
	glm::float32 zFar = 2000.0f;
	projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);

	static double lastTime = glfwGetTime();
	float time = 0.0f;
	float fTime = 0.0f;
	unsigned long frames = 0;

	bool hasInteracted = false;
	float cameraWobblePhase = 0.0f;
	const float WOBBLE_SPEED = 2.0f;
	const float WOBBLE_AMOUNT = 0.00005f;

	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);
		lastTime = currentTime;


		if (hasInteracted) {
			cameraWobblePhase += deltaTime * WOBBLE_SPEED;
			eye_center.y += sin(cameraWobblePhase) * WOBBLE_AMOUNT;
		}



		viewMatrix = glm::lookAt(eye_center, lookat, up);
		glm::mat4 vp = projectionMatrix * viewMatrix;

		viewMatrixSky = glm::mat4(glm::mat3(viewMatrix)); // Remove translation
		glm::mat4 vpSky = projectionMatrix * viewMatrixSky;
		//sky.render(vpSky);

		currentTime = glfwGetTime();

		//aircraft.render(vp, eye_center);
		//aircraft.update(currentTime);
		//tree.render(vp, eye_center);
		//b.render(vp, viewMatrix, projectionMatrix, eye_center);
		//ground.render(vp);
		// Render the building
		city.update(eye_center);
		city.render(vp, viewMatrix, projectionMatrix, eye_center);

		//obelisc.render(vp, eye_center);

		//bot.render(vp);

		//tree.render(vp, eye_center);

		//get the bots of the city
		//std::vector<std::shared_ptr<Entity>> bots;
		//for (const auto& bot : city.getBots()) {
		//	bots.push_back(std::static_pointer_cast<Entity>(bot));
		//}

		//update each bot
		//for (auto& bot : bots) {
		//	if (auto myBot = std::dynamic_pointer_cast<MyBot>(bot)) {
		//		myBot->update(currentTime);
		//	}
		//}


		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !hasInteracted) {
			auto [aircraftPos, aircraftFound] = city.handleAircraftInteraction(eye_center, viewMatrix, projectionMatrix);

			if (aircraftFound) {
				std::cout << "Aircraft found at: " << aircraftPos.x << ", " << aircraftPos.y << ", " << aircraftPos.z << std::endl;
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
			stream << std::fixed << std::setprecision(2) << "Lab 4 | Frames per second (FPS): " << fps;
		}
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

float mouseSensitivity = 0.5f;

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
    const float cameraSpeed = 20.0f;
    const float minHeight = 0.0f;
    const float maxHeight = 300.0f;

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

    // Height control with Left Control + Up/Down
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
            eye_center.y += cameraSpeed;
            eye_center.y = glm::clamp(eye_center.y, minHeight, maxHeight);
            lookat.y = eye_center.y;
        }
        if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
            eye_center.y -= cameraSpeed;
            eye_center.y = glm::clamp(eye_center.y, minHeight, maxHeight);
            lookat.y = eye_center.y;
        }
    }

    // Escape to close window
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);




}