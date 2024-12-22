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
#include <spire.h>
#include <obelisk.h>
#include <tea.h>


// OpenGL camera view parameters
static glm::vec3 eye_center(0.0f, -100.0f, 0.0f); // Position the camera inside the cube
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

struct LightPoint {
	glm::vec3 position;
	glm::vec3 color;
	float intensity;
};



struct InfiniteCity {

	// Templates de objetos base
	std::vector<std::shared_ptr<Building>> buildingTemplates;
	std::shared_ptr<Ground> groundTemplate;
	std::shared_ptr<Ground> roadGroundTemplate;

	std::shared_ptr<Aircraft> aircraftTemplate;
	std::shared_ptr<MyBot> botTemplate;

	std::shared_ptr<Obelisk> obeliskTemplate;
	std::shared_ptr<Spire> spireTemplate;
	std::shared_ptr<Tea> teaTemplate;

	std::shared_ptr<Tree> treeTemplate;


	std::unordered_map<ChunkCoord, std::vector<std::shared_ptr<Entity>>, ChunkCoordHash> currentChunks;



    const int CHUNK_SIZE = 192 *2;        // Size of chunk (6 buildings * (32*2 + 6))
    const int RENDER_DISTANCE = 8;      // How many chunks to render in each direction

	const float AIRCRAFT_SPAWN_PROBABILITY = 0.01f;

	std::vector<std::shared_ptr<Entity>> currentEntities;


	GLuint shadowMapFBO;
	GLuint shadowMapTexture;
	GLuint shadowProgramID;
	glm::mat4 lightSpaceMatrix;

	glm::vec3 lightPosition = glm::vec3(300.0f, 200.0f, 300.0f);
	glm::vec3 lightLookAt = glm::vec3(0.0f, -1.0f, 0.0f);

	bool saveDepth = false;

	static const int LIGHT_GROUP_SIZE = 1; // Cada luz cubre 3x3 chunks
	static constexpr float LIGHT_HEIGHT = 400.0f;
	std::unordered_map<ChunkCoord, LightPoint, ChunkCoordHash> lightPoints;

	GLFWwindow* glfwWindow = nullptr;

	bool aircraftFound = false;
	std::shared_ptr<Aircraft> currentInteractableAircraft = nullptr;


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
			"../lab2/textures/panel"
		);

		roadGroundTemplate = std::make_shared<Ground>();
		roadGroundTemplate->initialize(
			glm::vec3(0, 0, 0),
			glm::vec3(CHUNK_SIZE/2, 1, CHUNK_SIZE/2),
			"../lab2/textures/ground" );

		aircraftTemplate = std::make_shared<Aircraft>();
		aircraftTemplate->initialize(
			glm::vec3(0, 100, 0),
			glm::vec3(30, 30, 30)
		);

		botTemplate = std::make_shared<MyBot>();
		botTemplate->initialize(
			glm::vec3(0, 0, 0),
			glm::vec3(2,2,2),
			glm::vec3 (0,0,0)
		);

		obeliskTemplate = std::make_shared<Obelisk>();
		obeliskTemplate->initialize(
			glm::vec3(0, 0, 0),
			glm::vec3(5, 5, 8)
		);

		spireTemplate = std::make_shared<Spire>();
		spireTemplate->initialize(
			glm::vec3(0, 0, 0),
			glm::vec3(1, 1, 1)
		);

		teaTemplate = std::make_shared<Tea>();
		teaTemplate->initialize(
			glm::vec3(0, 0, 0),
			glm::vec3(15, 15, 15)
		);

		treeTemplate = std::make_shared<Tree>();
		treeTemplate->initialize(
			glm::vec3(0, 0, 0),
			glm::vec3(20, 20, 20)
		);

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
	generateLightForGroup(coord);
	std::seed_seq seed{coord.x, coord.z};
	std::mt19937 rng(seed);


    auto centerMarker = std::make_shared<Building>(*buildingTemplates[0]);
    centerMarker->setScale(glm::vec3(4, 4, 4));
    centerMarker->setPosition(glm::vec3(
        coord.x * CHUNK_SIZE + CHUNK_SIZE / 2,
        2,
        coord.z * CHUNK_SIZE + CHUNK_SIZE / 2
    ));
    chunkEntities.push_back(centerMarker);

    if (coord.x == 0 && coord.z == 0) {
        //initilize a centrar aircraft
		auto aircraftCentral = std::make_shared<Aircraft>(*aircraftTemplate);
    	aircraftCentral->initialize(
			glm::vec3(
				coord.x * CHUNK_SIZE + CHUNK_SIZE / 2,
				100,
				coord.z * CHUNK_SIZE + CHUNK_SIZE / 2
			),
			glm::vec3(30, 30, 30)
		);

    	chunkEntities.push_back(aircraftCentral);




        auto ground = std::make_shared<Ground>(*groundTemplate);
        ground->initialize(
            glm::vec3(coord.x * CHUNK_SIZE, 0, coord.z * CHUNK_SIZE),
            glm::vec3(CHUNK_SIZE, 1, CHUNK_SIZE),
            "../lab2/textures/ground"
        );
    	ground->setPosition(glm::vec3(
			coord.x * CHUNK_SIZE + CHUNK_SIZE / 2,
			0,
			coord.z * CHUNK_SIZE + CHUNK_SIZE / 2
		));
    	ground->setScale(glm::vec3(CHUNK_SIZE, 1, CHUNK_SIZE));

        chunkEntities.push_back(ground);

    	//render a tree
    	auto tree = std::make_shared<Tree>(*treeTemplate);
    	tree->setPosition(glm::vec3(
    	coord.x * CHUNK_SIZE + CHUNK_SIZE / 2,
			0,
			coord.z * CHUNK_SIZE + CHUNK_SIZE / 2
			));
    	chunkEntities.push_back(tree);


    	//render a bot
        auto bot = std::make_shared<MyBot>(*botTemplate);
    	bot->setPosition(glm::vec3(
    	coord.x * CHUNK_SIZE + CHUNK_SIZE / 2,
		0,
		coord.z * CHUNK_SIZE + CHUNK_SIZE / 2
		));
    	chunkEntities.push_back(bot);

    	// render the obelisk, spire and tea
    	auto obelisk = std::make_shared<Obelisk>(*obeliskTemplate);
    	obelisk->setPosition(glm::vec3(
			coord.x * CHUNK_SIZE + CHUNK_SIZE * 0.25f,
			0,
			coord.z * CHUNK_SIZE + CHUNK_SIZE * 0.5f
		));
    	chunkEntities.push_back(obelisk);

    	auto spire = std::make_shared<Spire>(*spireTemplate);
    	spire->setPosition(glm::vec3(
			0,
			-5,
			coord.z * CHUNK_SIZE + CHUNK_SIZE * 0.75f
		));
    	chunkEntities.push_back(spire);

    	auto tea = std::make_shared<Tea>(*teaTemplate);
    	tea->setPosition(glm::vec3(
			coord.x * CHUNK_SIZE + CHUNK_SIZE * 0.25f,
			0,
			coord.z * CHUNK_SIZE + CHUNK_SIZE * 0.25f
		));
    	chunkEntities.push_back(tea);
    }

    else if (std::abs(coord.x) <= 2 && std::abs(coord.z) <= 2) {
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

        float height = 300.0f + (rng() % 300);

        if (rng() % 100 < AIRCRAFT_SPAWN_PROBABILITY * 100) {
        	auto aircraft = std::make_shared<Aircraft>(*aircraftTemplate);
        	glm::vec3 aircraftPos = glm::vec3(
				coord.x * CHUNK_SIZE + (rand() % CHUNK_SIZE),
				height,
				coord.z * CHUNK_SIZE + (rand() % CHUNK_SIZE)
			);
        	aircraft->setPosition(aircraftPos);
        	//set a random orientation
        	aircraft->setRotation(glm::vec3(0, rand() % 360, 0));
        	chunkEntities.push_back(aircraft);
        }


        } else {
            auto ground = std::make_shared<Ground>(*groundTemplate);
            ground->setPosition(glm::vec3(
                coord.x * CHUNK_SIZE + CHUNK_SIZE / 2,
                0,
                coord.z * CHUNK_SIZE + CHUNK_SIZE / 2
            ));
            chunkEntities.push_back(ground);

            int blockX = std::abs(coord.x) % 3;
            int blockZ = std::abs(coord.z) % 3;


			const int numCrystals = 5 + (rng() % 10);
			const float centerArea = CHUNK_SIZE * 0.3f;
			const float baseWidth = 16.0f;

			std::vector<glm::vec2> crystalPositions;


			glm::vec2 centerPos(
			    coord.x * CHUNK_SIZE + CHUNK_SIZE / 2,
			    coord.z * CHUNK_SIZE + CHUNK_SIZE / 2
			);
			crystalPositions.push_back(centerPos);

			float firstHeight = 64.0f + (rng() % 128);
			float firstScaleX = 0.5f + (float)(rng() % 100) / 100.0f;
			float firstScaleZ = 0.5f + (float)(rng() % 100) / 100.0f; // Escala en Z entre 0.5 y 1.5

			auto firstCrystal = std::make_shared<Building>(*buildingTemplates[0]);
			firstCrystal->setScale(glm::vec3(baseWidth * firstScaleX, firstHeight, baseWidth * firstScaleZ));
			firstCrystal->setPosition(glm::vec3(centerPos.x, firstHeight , centerPos.y));
			firstCrystal->setRotation(glm::vec3(0, 0, 0)); // Rotación aleatoria en Y
			chunkEntities.push_back(firstCrystal);

			// Generar el resto de los cristales alrededor del primero
			for (int i = 1; i < numCrystals; i++) {
			    float randomHeight = 64.0f + (rng() % 128); // Altura aleatoria entre 64 y 192
			    float randomScaleX = 0.5f + (float)(rng() % 100) / 100.0f; // Escala en X entre 0.5 y 1.5
			    float randomScaleZ = 0.5f + (float)(rng() % 100) / 100.0f; // Escala en Z entre 0.5 y 1.5
			    auto crystal = std::make_shared<Building>(*buildingTemplates[0]);

			    // Generar una posición ligeramente alejada del centro
			    float angle = (float)(rng() % 360) * 3.14159f / 180.0f; // Ángulo aleatorio
			    float distance = (float)(rng() % (int)(centerArea)); // Distancia aleatoria dentro del área central

			    float offsetX = cos(angle) * distance;
			    float offsetZ = sin(angle) * distance;

			    glm::vec2 pos(
			        centerPos.x + offsetX,
			        centerPos.y + offsetZ
			    );

			    // Añadir rotación aleatoria
			    glm::vec3 randomRotation(
			        0, // No rotación en X
			        rng() % 30, // Rotación aleatoria en Y
			        rng() % 5 // Ligeras inclinaciones en Z para variación
			    );

			    // Configurar el cristal
			    crystal->setScale(glm::vec3(baseWidth * randomScaleX, randomHeight, baseWidth * randomScaleZ));
			    crystal->setPosition(glm::vec3(pos.x, randomHeight, pos.y));
			    crystal->setRotation(randomRotation);
			    chunkEntities.push_back(crystal);
			}
        	glm::vec3 positions[4];


			// Definir las cuatro posiciones posibles (bordes del chunk)
			positions[0] = glm::vec3(coord.x * CHUNK_SIZE + (rng() % 10), 0, coord.z * CHUNK_SIZE);                    // Norte
			positions[1] = glm::vec3((coord.x + 1) * CHUNK_SIZE, 0, coord.z * CHUNK_SIZE + (rng() % 10));             // Este
			positions[2] = glm::vec3(coord.x * CHUNK_SIZE + (rng() % 10), 0, (coord.z + 1) * CHUNK_SIZE - (rng() % 10)); // Sur
			positions[3] = glm::vec3(coord.x * CHUNK_SIZE, 0, coord.z * CHUNK_SIZE + (rng() % 10));                   // Oeste

			// Elegir dos posiciones al azar
			std::vector<int> selectedIndices = {0, 1, 2, 3};
			std::shuffle(selectedIndices.begin(), selectedIndices.end(), rng);

			int firstIndex = selectedIndices[0];
			int secondIndex = selectedIndices[1];

			// Decidir si en la primera posición habrá un bot (40%) o un tree (20%)
			int firstChance = rng() % 10;
			if (firstChance < 3) {
			    // Bot en la primera posición
			    auto bot = std::make_shared<MyBot>(*botTemplate);
			    bot->setPosition(positions[firstIndex]);
				bot->setRotation(glm::vec3(0, rand() % 360, 0));
			    chunkEntities.push_back(bot);
			} else if (firstChance < 4) {
			    // Tree en la primera posición
			    auto tree = std::make_shared<Tree>(*treeTemplate);


			    tree->setPosition(positions[firstIndex]);
			    chunkEntities.push_back(tree);
			}

			// Decidir si en la segunda posición habrá un bot (40%) o un tree (20%)
			int secondChance = rng() % 10;
			bool secondIsBot = (secondChance < 3);
			bool secondIsTree = (secondChance >= 3 && secondChance < 4);

			if (secondIsBot) {
			    // Bot en la segunda posición
			    auto bot = std::make_shared<MyBot>(*botTemplate);
			    bot->setPosition(positions[secondIndex]);
				bot->setRotation(glm::vec3(0, rand() % 360, 0));


			    // Verificar si la posición está ocupada
			    if (glm::distance(glm::vec2(positions[firstIndex].x, positions[firstIndex].z), glm::vec2(positions[secondIndex].x, positions[secondIndex].z)) >= 5.0f) {
			        chunkEntities.push_back(bot);
			    }
			} else if (secondIsTree) {
			    // Tree en la segunda posición
			    auto tree = std::make_shared<Tree>(*treeTemplate);

			    float randomHeight = 15.0f + (rng() % 5); // Altura entre 20 y 40
			    tree->setScale(glm::vec3(15.0f, randomHeight, 15.0f));
			    tree->setPosition(positions[secondIndex]);

			    // Verificar si la posición está ocupada
			    if (glm::distance(glm::vec2(positions[firstIndex].x, positions[firstIndex].z), glm::vec2(positions[secondIndex].x, positions[secondIndex].z)) >= 5.0f) {
			        chunkEntities.push_back(tree);
			    }
			}




        }
    }

    currentChunks[coord] = chunkEntities;
}

	void update(const glm::vec3& cameraPos, double currentTime) {
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

		for (auto& [coord, entities] : currentChunks) {
			for (auto& entity : entities) {
				if (auto bot = std::dynamic_pointer_cast<MyBot>(entity)) {
					bot->update(currentTime);

				}
			}
		}

		cleanupDistantChunks(currentChunk);
	}

	void render(glm::mat4 vp, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, glm::vec3 eye) {



		for (const auto& [coord, chunk] : currentChunks) {

			LightPoint light = getClosestLight(coord);

			auto centerMarker = std::make_shared<Building>(*buildingTemplates[0]);
			centerMarker->setScale(glm::vec3(4, 4, 4));
			centerMarker->setPosition(light.position);


			for (auto& entity : chunk) {
				if (auto bot = std::dynamic_pointer_cast<MyBot>(entity)) {
					bot->setLightUniforms(light.position, light.color, eye, light.intensity);
					bot->render(vp);
				} else if (auto ground = std::dynamic_pointer_cast<Ground>(entity)) {
					ground->setLightUniforms(light.position, light.color, eye, light.intensity);
					ground->render(vp, 0, glm::mat4(1.0f));
				} else if (auto aircraft = std::dynamic_pointer_cast<Aircraft>(entity)) {
					aircraft->setLightUniforms(light.position, light.color, eye, light.intensity);
					aircraft->render(vp, eye);
				} else if (auto building = std::dynamic_pointer_cast<Building>(entity)) {
					building->setLightUniforms(light.position, light.color, eye, light.intensity);
					building->render(vp, viewMatrix, projectionMatrix, eye, 0, glm::mat4(1.0f));
				} else if (auto obelisk = std::dynamic_pointer_cast<Obelisk>(entity)) {
					obelisk->setLightUniforms(light.position, light.color, eye, light.intensity);
					obelisk->render(vp, eye);
				} else if (auto spire = std::dynamic_pointer_cast<Spire>(entity)) {
					spire->setLightUniforms(light.position, light.color, eye, light.intensity);
					spire->render(vp, eye);
				} else if (auto tea = std::dynamic_pointer_cast<Tea>(entity)) {
					tea->setLightUniforms(light.position, light.color, eye, light.intensity);
					tea->render(vp, eye);
				} else if (auto tree = std::dynamic_pointer_cast<Tree>(entity)) {
					tree->setLightUniforms(light.position, light.color, eye, light.intensity);
					tree->render(vp, eye);
				}
				centerMarker->render(vp, viewMatrix, projectionMatrix, eye, 0, glm::mat4(1.0f));
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

	ChunkCoord getLightGroupCoord(const ChunkCoord& coord) {
		return {
			(coord.x / LIGHT_GROUP_SIZE) * LIGHT_GROUP_SIZE,
			(coord.z / LIGHT_GROUP_SIZE) * LIGHT_GROUP_SIZE
		};
	}

	void generateLightForGroup(const ChunkCoord& coord) {
		// Obtener coordenada del grupo
		ChunkCoord groupCoord = getLightGroupCoord(coord);

		// Si ya existe una luz para este grupo, no hacer nada
		if (lightPoints.find(groupCoord) != lightPoints.end()) {
			return;
		}

		// Crear nuevo punto de luz para el grupo
		LightPoint light;
		light.position = glm::vec3(
			groupCoord.x * CHUNK_SIZE + (CHUNK_SIZE * LIGHT_GROUP_SIZE / 2),
			LIGHT_HEIGHT - 200,
			groupCoord.z * CHUNK_SIZE + (CHUNK_SIZE * LIGHT_GROUP_SIZE / 2)
		);
		light.color = glm::vec3(1.0f, 1.0f, 1.0f);
		light.intensity = 1.0f;

		lightPoints[groupCoord] = light;
	}

	LightPoint getClosestLight(const ChunkCoord& coord) {
		return lightPoints[getLightGroupCoord(coord)];
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
		bool found = false;
		const float INTERACTION_CUBE_SIZE = 100.0f; // Size of the interaction cube

		for (const auto& [_, chunk] : currentChunks) {
			for (auto& entity : chunk) {
				if (auto aircraft = std::dynamic_pointer_cast<Aircraft>(entity)) {
					glm::vec3 aircraftPos = aircraft->getPosition();

					// Check if camera is inside the interaction cube
					bool insideCube =
						cameraPos.x >= aircraftPos.x - INTERACTION_CUBE_SIZE &&
						cameraPos.x <= aircraftPos.x + INTERACTION_CUBE_SIZE &&
						cameraPos.y >= aircraftPos.y - INTERACTION_CUBE_SIZE &&
						cameraPos.y <= aircraftPos.y + INTERACTION_CUBE_SIZE &&
						cameraPos.z >= aircraftPos.z - INTERACTION_CUBE_SIZE &&
						cameraPos.z <= aircraftPos.z + INTERACTION_CUBE_SIZE;

					if (insideCube) {
						aircraft->setInteractable(true);
						currentInteractableAircraft = aircraft;
						found = true;
						return aircraft;
					}
				}
			}
		}

		// If no aircraft was found in range, disable current interactable
		if (!found && currentInteractableAircraft) {
			currentInteractableAircraft->setInteractable(false);
			currentInteractableAircraft = nullptr;
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
			if ((std::abs(coord.x - currentChunk.x) > RENDER_DISTANCE + 1 ||
				std::abs(coord.z - currentChunk.z) > RENDER_DISTANCE + 1) &&
					!(coord.x == 0 && coord.z == 0))
				{
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