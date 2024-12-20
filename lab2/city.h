#ifndef CITY_H
#define CITY_H

#include "TextRenderer.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <cstdlib>
#include <ctime>

#include <render/shader.h>


#include <vector>
#include <iostream>


#include <math.h>
#include <unordered_map>
#include <cmath>
#include <fastNoiseLite.h>
#include <random>


#include <building.h>
#include <ground.h>
#include <memory>
#include <tree.h>
#include <aircraft.h>
#include <entity.h>
#include <sky.h>
#include <bot.h>
#include <stb_image_write.h>



// OpenGL camera view parameters
static glm::vec3 eye_center(300.0f, 600.0f, 300.0f); // Position the camera inside the cube
static glm::vec3 lookat(0, 1, 1);
static glm::vec3 up(0, 1, 0);

// View control
float viewPolar = glm::radians(60.0f);
float viewAzimuth = glm::radians(45.0f);
static float viewDistance = 400.0f;

glm::vec3 front = glm::normalize(lookat - eye_center);

static void saveDepthTexture(GLuint fbo, std::string filename) {
	int width = 1024;
	int height = 1024;

	int channels = 3;

	std::vector<float> depth(width * height);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glReadBuffer(GL_DEPTH_COMPONENT);
	glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth.data());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	std::vector<unsigned char> img(width * height * 3);
	for (int i = 0; i < width * height; ++i) img[3*i] = img[3*i+1] = img[3*i+2] = depth[i] * 255;

	stbi_write_png(filename.c_str(), width, height, channels, img.data(), width * channels);
}

// Chunk coordinate structure definition
struct ChunkCoord {
    int x;
    int z;

    bool operator==(const ChunkCoord& other) const {
        return x == other.x && z == other.z;
    }
};

struct ChunkData {
	float density;
	float variation;
	std::vector<std::shared_ptr<Entity>> entities;
};


// Hash function structure for ChunkCoord
struct ChunkCoordHash {
    std::size_t operator()(const ChunkCoord& coord) const {
        return std::hash<int>()(coord.x) ^ (std::hash<int>()(coord.z) << 1);
    }
};



struct InfiniteCity {

	// Templates de objetos base
	std::vector<std::shared_ptr<Building>> buildingTemplates;
	std::shared_ptr<Ground> groundTemplate;
	std::shared_ptr<Ground> roadGroundTemplate;

	std::shared_ptr<Aircraft> aircraftTemplate;
	std::shared_ptr<MyBot> botTemplate;


	std::unordered_map<ChunkCoord, std::vector<std::shared_ptr<Entity>>, ChunkCoordHash> currentChunks;



    const int CHUNK_SIZE = 192 *2;        // Size of chunk (6 buildings * (32*2 + 6))
    const int RENDER_DISTANCE = 8;      // How many chunks to render in each direction
    const float STREET_WIDTH = 6.0f;    // Width of streets between buildings
    const int BUILDINGS_PER_ROW = 1;    // Number of buildings per row in a chunk
	const float AIRCRAFT_SPAWN_PROBABILITY = 0.0f; // 50% chance of spawning an aircraft

	std::vector<std::shared_ptr<Entity>> currentEntities;


	GLuint shadowMapFBO;
	GLuint shadowMapTexture;
	GLuint shadowProgramID;
	glm::mat4 lightSpaceMatrix;

	glm::vec3 lightPosition = glm::vec3(300.0f, 200.0f, 300.0f);
	glm::vec3 lightLookAt = glm::vec3(0.0f, -1.0f, 0.0f);

	bool saveDepth = false;

	GLFWwindow* glfwWindow = nullptr;

	bool aircraftFound = false;
	std::shared_ptr<Aircraft> currentInteractableAircraft = nullptr;





    // Texture paths array
	// Texture paths array
	const char* texturePaths[6] = {
		"../lab2/textures/cube_right.png",
		"../lab2/textures/cube_right.png",
		"../lab2/textures/cube_right.png",
		"../lab2/textures/cube_right.png",
		"../lab2/textures/cube_right.png",
		"../lab2/textures/cube_right.png",
	};

	const char* groundTexturePaths[6] = {
		"../lab2/textures/cube_right.png",
		"../lab2/textures/cube_right.png",
		"../lab2/textures/cube_right.png",
		"../lab2/textures/cube_right.png",
		"../lab2/textures/cube_right.png",
		"../lab2/textures/cube_right.png",
	};

	// Map of chunks to entities

	std::unordered_map<ChunkCoord, std::vector<std::shared_ptr<Entity>>, ChunkCoordHash> chunks;

	InfiniteCity(GLFWwindow* window) : glfwWindow(window)
	{
		initializeTemplates();
	}

	void initializeTemplates() {
		buildingTemplates.push_back(std::make_shared<Building>());
		buildingTemplates[0]->initialize(
			glm::vec3(0, 0, 0),
			glm::vec3(32, 32, 32),
			"../lab2/textures/cube_"
		);

		groundTemplate = std::make_shared<Ground>();
		groundTemplate->initialize(
			glm::vec3(0, 0, 0),
			glm::vec3(CHUNK_SIZE/2, 1, CHUNK_SIZE/2),
			"../lab2/textures/facade2.jpg"
		);

		roadGroundTemplate = std::make_shared<Ground>();
		roadGroundTemplate->initialize(
			glm::vec3(0, 0, 0),
			glm::vec3(CHUNK_SIZE/2, 1, CHUNK_SIZE/2),
			"../lab2/textures/cube_up.png" );

		aircraftTemplate = std::make_shared<Aircraft>();
		aircraftTemplate->initialize(
			glm::vec3(0, 100, 0),
			glm::vec3(20, 20, 20)
		);

		//botTemplate = std::make_shared<MyBot>();
		//botTemplate->initialize(
		//	glm::vec3(0, 0, 0),
		//	glm::vec3(1, 1, 1)
		//);
	}

	ChunkCoord getCurrentChunk(const glm::vec3& cameraPos) {
		return {
			static_cast<int>(std::floor(cameraPos.x / CHUNK_SIZE)),
			static_cast<int>(std::floor(cameraPos.z / CHUNK_SIZE))
		};
	}

    ChunkCoord getChunkCoord(float x, float z) {
        return {
            static_cast<int>(std::floor(x / CHUNK_SIZE)),
            static_cast<int>(std::floor(z / CHUNK_SIZE))
        };
    }


void generateChunk(const ChunkCoord& coord) {
    std::vector<std::shared_ptr<Entity>> chunkEntities;

    // Añadir marcador negro en el centro del chunk
    auto centerMarker = std::make_shared<Building>(*buildingTemplates[0]);
    centerMarker->setScale(glm::vec3(4, 4, 4)); // Pequeño cubo de 4x4x4
    centerMarker->setPosition(glm::vec3(
        coord.x * CHUNK_SIZE + CHUNK_SIZE / 2,
        2, // Ligeramente elevado para ser visible
        coord.z * CHUNK_SIZE + CHUNK_SIZE / 2
    ));
    // Hacer el marcador negro
    chunkEntities.push_back(centerMarker);

    // Manejo del chunk de spawn (centro)
    if (coord.x == 0 && coord.z == 0) {
        auto aircraft = std::make_shared<Aircraft>(*aircraftTemplate);
        aircraft->setPosition(glm::vec3(
            coord.x * CHUNK_SIZE + CHUNK_SIZE / 2,
            100,
            coord.z * CHUNK_SIZE + CHUNK_SIZE / 2
        ));
        chunkEntities.push_back(aircraft);

        auto ground = std::make_shared<Ground>(*groundTemplate);
        ground->initialize(
            glm::vec3(coord.x * CHUNK_SIZE, 0, coord.z * CHUNK_SIZE),
            glm::vec3(CHUNK_SIZE, 1, CHUNK_SIZE),
            "../lab2/textures/AIRGROUND.png"
        );
        ground->setPosition(glm::vec3(
            coord.x * CHUNK_SIZE + CHUNK_SIZE / 2,
            0,
            coord.z * CHUNK_SIZE + CHUNK_SIZE / 2
        ));
        chunkEntities.push_back(ground);
    }
    // Área reservada alrededor del spawn
    else if (std::abs(coord.x) <= 4 && std::abs(coord.z) <= 4) {
        return;
    }
    else {
        bool isRoad = ((coord.x) % 3 == 0) || ((coord.z) % 3 == 0);

        if (isRoad) {
            auto road = std::make_shared<Ground>(*roadGroundTemplate);
            road->setPosition(glm::vec3(
                coord.x * CHUNK_SIZE + CHUNK_SIZE / 2,
                0,
                coord.z * CHUNK_SIZE + CHUNK_SIZE / 2
            ));
            chunkEntities.push_back(road);
        } else {
            auto ground = std::make_shared<Ground>(*groundTemplate);
            ground->setPosition(glm::vec3(
                coord.x * CHUNK_SIZE + CHUNK_SIZE / 2,
                0,
                coord.z * CHUNK_SIZE + CHUNK_SIZE / 2
            ));
            chunkEntities.push_back(ground);

            // Calculamos la posición dentro de la manzana
            int blockX = std::abs(coord.x) % 3;
            int blockZ = std::abs(coord.z) % 3;

            // Usamos las coordenadas como seed para la generación consistente
            std::seed_seq seed{coord.x, coord.z};
            std::mt19937 rng(seed);

            // Calculamos el número de edificios y su espaciado
            const int BUILDINGS_IN_CHUNK = 4;
            const float BUILDING_SPACING = (CHUNK_SIZE - STREET_WIDTH * 2) / 2;

            // Generamos edificios en una cuadrícula 2x2
            for (int x = 0; x < 2; x++) {
                for (int z = 0; z < 2; z++) {
                    float randomHeight = 32.0f + (rng() % 96);
                    auto building = std::make_shared<Building>(*buildingTemplates[0]);

                    float posX = coord.x * CHUNK_SIZE + STREET_WIDTH + x * BUILDING_SPACING + BUILDING_SPACING/2;
                    float posZ = coord.z * CHUNK_SIZE + STREET_WIDTH + z * BUILDING_SPACING + BUILDING_SPACING/2;

                    building->setScale(glm::vec3(32, randomHeight, 32));
                    building->setPosition(glm::vec3(
                        posX,
                        randomHeight,
                        posZ
                    ));

                    chunkEntities.push_back(building);
                }
            }
        }
    }

    currentChunks[coord] = chunkEntities;
}

	void update(const glm::vec3& cameraPos) {

		ChunkCoord currentChunk = getCurrentChunk(cameraPos);

		for (int dx = -RENDER_DISTANCE; dx <= RENDER_DISTANCE; ++dx) {
			for (int dz = -RENDER_DISTANCE; dz <= RENDER_DISTANCE; ++dz) {
				ChunkCoord chunkToLoad = {
					currentChunk.x + dx,
					currentChunk.z + dz
				};

				if (currentChunks.find(chunkToLoad) == currentChunks.end()) {
					generateChunk(chunkToLoad);
				}
			}
		}


	}

	void render(glm::mat4 vp, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, glm::vec3 eye) {
		for (const auto& [_, chunk] : currentChunks) {
			for (auto& entity : chunk) {
				if (auto bot = std::dynamic_pointer_cast<MyBot>(entity)) {
					//bot->render(vp);
				} else if (auto ground = std::dynamic_pointer_cast<Ground>(entity)) {
					ground->render(vp, 0, glm::mat4(1.0f));
				} else if (auto aircraft = std::dynamic_pointer_cast<Aircraft>(entity)) {
					aircraft->render(vp, eye);
				} else if (auto building = std::dynamic_pointer_cast<Building>(entity)) {
					building->render(vp, viewMatrix, projectionMatrix, eye, 0, glm::mat4(1.0f));
				}
			}
		}

		findInteractableAircraftInFrustum(eye, projectionMatrix, viewMatrix);

		// Save depth texture if enabled
		if (saveDepth) {
			std::string filename = "depth_camera.png";
			saveDepthTexture(shadowMapFBO, filename);
			std::cout << "Depth texture saved to " << filename << std::endl;
			saveDepth = false;
		}
	}

	void initializeShadowMapping() {
        glGenFramebuffers(1, &shadowMapFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

        glGenTextures(1, &shadowMapTexture);
        glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        shadowProgramID = LoadShadersFromFile("..lab2/shaders/shadow.vert", "..lab2/shaders/shadow.frag");

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTexture, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

    	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    	if (status != GL_FRAMEBUFFER_COMPLETE) {
    		std::cerr << "Error framebuffer: " << status << std::endl;
    	}

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void renderShadowMap(glm::vec3 lightPos, glm::vec3 lightLookAt) {
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
        glViewport(0, 0, 1024, 1024);
        glClear(GL_DEPTH_BUFFER_BIT);

        glUseProgram(shadowProgramID);

        glm::mat4 lightProjection = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 100.0f);
        glm::mat4 lightView = glm::lookAt(lightPos, lightLookAt, glm::vec3(0.0f, 1.0f, 0.0f));
        lightSpaceMatrix = lightProjection * lightView;

        GLuint lightSpaceMatrixID = glGetUniformLocation(shadowProgramID, "lightSpaceMatrix");
        glUniformMatrix4fv(lightSpaceMatrixID, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

	    for (auto& chunk : chunks) {
	        for (auto& entity : chunk.second) {
				entity->renderForShadows(lightSpaceMatrix, shadowProgramID);
	        	// print the position of each entity


	        }
	    }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    std::shared_ptr<Aircraft> findInteractableAircraftInFrustum(
        const glm::vec3& cameraPos,
        const glm::mat4& projectionMatrix,
        const glm::mat4& viewMatrix
    ) {
        Frustum frustum;
        glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
        frustum.extractFrustumPlanes(viewProjectionMatrix);

		bool found = false;

        for (const auto& [_, chunk] : currentChunks) {
            for (auto& entity : chunk) {
                if (auto aircraft = std::dynamic_pointer_cast<Aircraft>(entity)) {
                    glm::vec3 aircraftPos = aircraft->getPosition();
                    float aircraftRadius = 20.0f;

                    if (frustum.sphereInFrustum(aircraftPos, aircraftRadius)) {
                        glm::vec3 toAircraft = glm::normalize(aircraftPos - cameraPos);
                        float angle = glm::degrees(glm::acos(glm::dot(front, toAircraft)));

                        if (angle < 15.0f) {
                            aircraft->setInteractable(true);
                        	currentInteractableAircraft = aircraft;
                            return aircraft;
                        }
                    }
                }
            }
        }
		if (!found) {
			if (currentInteractableAircraft) {
				currentInteractableAircraft->setInteractable(false);
				currentInteractableAircraft = nullptr;
			}
		}

        return nullptr;
    }

	std::pair<glm::vec3, bool> handleAircraftInteraction(
		const glm::vec3& cameraPos,
		const glm::mat4& viewMatrix,
		const glm::mat4& projectionMatrix
	)
	{
		currentInteractableAircraft = findInteractableAircraftInFrustum(
			cameraPos, projectionMatrix, viewMatrix
		);

		if (currentInteractableAircraft && !aircraftFound) {
			currentInteractableAircraft->onInteract();
			aircraftFound = true;
			return {currentInteractableAircraft->getPosition(), true};
		} else {
			return {glm::vec3(0.0f), false};
		}
	}

	void cleanup() {
    	for (auto& chunk : chunks) {
    		for (auto& entity : chunk.second) {
    			entity->cleanup();
    		}
    	}
    	chunks.clear();


    }

	void cleanupDistantChunks(const ChunkCoord& currentChunk) {
		std::vector<ChunkCoord> chunksToRemove;

		for (const auto& [coord, _] : currentChunks) {
			if (std::abs(coord.x - currentChunk.x) > RENDER_DISTANCE + 1 ||
				std::abs(coord.z - currentChunk.z) > RENDER_DISTANCE + 1) {
				chunksToRemove.push_back(coord);
				}
		}

		for (const auto& coord : chunksToRemove) {
			currentChunks.erase(coord);
		}
	}

	//methos that returns all the bots in the city
	std::vector<std::shared_ptr<MyBot>> getBots() {
		std::vector<std::shared_ptr<MyBot>> bots;
		for (const auto& [_, chunk] : currentChunks) {
			for (auto& entity : chunk) {
				if (auto bot = std::dynamic_pointer_cast<MyBot>(entity)) {
					bots.push_back(bot);
				}
			}
		}
		return bots;
	}
};



#endif