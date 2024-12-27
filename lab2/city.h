#ifndef CITY_H
#define CITY_H


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
#include <bot.h>
#include <stb_image_write.h>
#include <spire.h>
#include <obelisk.h>
#include <tea.h>
#include <sphere.h>
#include <sky.h>



// OpenGL camera view parameters
static glm::vec3 eye_center(0.0f, -100.0f, 0.0f); // Position the camera inside the cube
static glm::vec3 lookat(0, 1, 1);
static glm::vec3 up(0, 1, 0);

// View control
float viewPolar = glm::radians(60.0f);
float viewAzimuth = glm::radians(45.0f);
static float viewDistance = 400.0f;

glm::vec3 front = glm::normalize(lookat - eye_center);


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

bool isRoadFunc(const ChunkCoord& coord) {
    const int PATTERN_SIZE = 3;

    bool isMainRoad = (coord.x % PATTERN_SIZE == 0) || (coord.z % PATTERN_SIZE == 0);
    if (isMainRoad) return true;

    // Determine which block we're in
    int blockX = coord.x / PATTERN_SIZE;
    int blockZ = coord.z / PATTERN_SIZE;

    uint32_t seed = blockX * 74531 + blockZ * 19477;
    float random = float(seed % 100) / 100.0f;

    // Determine block type based on random value
    enum BlockType {
        BLOCK_1x1 = 0,
        BLOCK_1x2 = 1,
        BLOCK_2x1 = 2,
        BLOCK_2x2 = 3
    };

    BlockType blockType;
    if (random < 0.25f) blockType = BLOCK_1x1;
    else if (random < 0.5f) blockType = BLOCK_1x2;
    else if (random < 0.75f) blockType = BLOCK_2x1;
    else blockType = BLOCK_2x2;

    int localX = coord.x % PATTERN_SIZE;
    int localZ = coord.z % PATTERN_SIZE;

    // Calculate if this should be a road based on block type
    switch (blockType) {
        case BLOCK_1x1:
            return (localX == 0) || (localZ == 0) ||
                   (localX == 2) || (localZ == 2);

        case BLOCK_1x2:
            if (random < 0.375f) {
                return (localX == 0) || (localZ == 0) ||
                       (localX == 2) || (localZ == 2);
            } else {
                return (localX == 0) || (localX == 2) ||
                       (localZ == 0);
            }

        case BLOCK_2x1:
            if (random < 0.625f) {
                return (localX == 0) || (localZ == 0) ||
                       (localZ == 2);
            } else {
                return (localX == 0) || (localZ == 0) ||
                       (localZ == 2);
            }

        case BLOCK_2x2:
            return (localX == 0) || (localZ == 0);

        default:
            return true;
    }
}

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
	std::shared_ptr<Sphere> sphereTemplate;

	std::shared_ptr<Aircraft> centralAircraftTemplate;




	std::unordered_map<ChunkCoord, std::vector<std::shared_ptr<Entity>>, ChunkCoordHash> currentChunks;



    const int CHUNK_SIZE = 192 * 2;        // Size of chunk
    const int RENDER_DISTANCE = 10;      // How many chunks to render in each direction

	const float AIRCRAFT_SPAWN_PROBABILITY = 0.04f;

	std::vector<std::shared_ptr<Entity>> currentEntities;


	GLuint shadowMapFBO;
	GLuint shadowMapTexture;
	GLuint shadowProgramID;
	glm::mat4 lightSpaceMatrix;

	glm::vec3 lightPosition = glm::vec3(300.0f, 200.0f, 300.0f);
	glm::vec3 lightLookAt = glm::vec3(0.0f, -1.0f, 0.0f);

	bool saveDepth = false;

	static const int LIGHT_GROUP_SIZE = 1;
	static constexpr float LIGHT_HEIGHT = 400.0f;
	std::unordered_map<ChunkCoord, LightPoint, ChunkCoordHash> lightPoints;

	GLFWwindow* glfwWindow = nullptr;

	bool aircraftFound = false;
	std::shared_ptr<Aircraft> currentInteractableAircraft = nullptr;


	std::unordered_map<ChunkCoord, std::vector<std::shared_ptr<Entity>>, ChunkCoordHash> chunks;

	InfiniteCity(GLFWwindow* window) : glfwWindow(window)
	{
		initializeTemplates();
	}

	// initializes the templates for the objects that will be used in the city only once
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
			"../lab2/textures/bricks" );

		aircraftTemplate = std::make_shared<Aircraft>();
		aircraftTemplate->initialize(
			glm::vec3(0, 100, 0),
			glm::vec3(30, 30, 30)
		);

		botTemplate = std::make_shared<MyBot>();
		botTemplate->initialize(
			glm::vec3(0, 0, 0),
			glm::vec3(1,1,1),
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

		sphereTemplate = std::make_shared<Sphere>();
		sphereTemplate->initialize(
			glm::vec3(0, 0, 0),
			glm::vec3(2, 2, 2)
		);

		centralAircraftTemplate = std::make_shared<Aircraft>(*aircraftTemplate);
		centralAircraftTemplate->initialize(
			glm::vec3(0, 0, 0),
			glm::vec3(30, 30, 30),
			true
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
	    // Initialize vector to store entities and random number generator
	    std::vector<std::shared_ptr<Entity>> chunkEntities;
	    std::seed_seq seed{coord.x, coord.z};
	    std::mt19937 rng(seed);

	    // Generate central chunk (0,0)
	    if (coord.x == 0 && coord.z == 0) {
	        generateLightForGroup(coord, chunkEntities);
	        auto road = std::make_shared<Ground>(*roadGroundTemplate);
	        road->setPosition(glm::vec3(coord.x * CHUNK_SIZE + CHUNK_SIZE / 2, 0, coord.z * CHUNK_SIZE + CHUNK_SIZE / 2));
	        chunkEntities.push_back(road);
	    }
	    // Generate surrounding chunks
	    else if (std::abs(coord.x) <= 1 && std::abs(coord.z) <= 1) {
	        generateLightForGroup(coord, chunkEntities);
	        auto road = std::make_shared<Ground>(*roadGroundTemplate);
	        road->setPosition(glm::vec3(coord.x * CHUNK_SIZE + CHUNK_SIZE / 2, 0, coord.z * CHUNK_SIZE + CHUNK_SIZE / 2));
	        chunkEntities.push_back(road);

	        const float OFFSET = CHUNK_SIZE * 0.3f;

	        // Place specific landmarks based on coordinates
	        if (coord.x == -1 && coord.z == -1) {
	            auto spire = std::make_shared<Spire>(*spireTemplate);
	            spire->setPosition(glm::vec3(coord.x * CHUNK_SIZE, -30, coord.z * CHUNK_SIZE));
	            chunkEntities.push_back(spire);
	        } else if (coord.x == -1 && coord.z == 1) {
	            auto tea = std::make_shared<Tea>(*teaTemplate);
	            tea->setPosition(glm::vec3(coord.x * CHUNK_SIZE + OFFSET, 10, coord.z * CHUNK_SIZE + CHUNK_SIZE - OFFSET));
	            chunkEntities.push_back(tea);
	        } else if (coord.x == 1 && coord.z == -1) {
	            auto obelisk = std::make_shared<Obelisk>(*obeliskTemplate);
	            obelisk->setPosition(glm::vec3(coord.x * CHUNK_SIZE + CHUNK_SIZE - OFFSET, 0, coord.z * CHUNK_SIZE + OFFSET));
	            chunkEntities.push_back(obelisk);
	        } else if (coord.x == 1 && coord.z == 1) {
	            auto aircraftCentral = std::make_shared<Aircraft>(*centralAircraftTemplate);
                aircraftCentral->setPosition(glm::vec3(coord.x * CHUNK_SIZE + CHUNK_SIZE / 2, 100, coord.z * CHUNK_SIZE + CHUNK_SIZE / 2));
	        	chunkEntities.push_back(aircraftCentral);
	        }
	    }
	    // Generate city chunks
	    else
	    {
		    bool isRoad = isRoadFunc(coord);

    		if (isRoad) {
    			generateLightForGroup(coord, chunkEntities);
    			auto road = std::make_shared<Ground>(*roadGroundTemplate);
    			road->setPosition(glm::vec3(coord.x * CHUNK_SIZE + CHUNK_SIZE / 2, 0, coord.z * CHUNK_SIZE + CHUNK_SIZE / 2));
    			chunkEntities.push_back(road);

    			// Randomly spawn an aircraft
    			float height = 300.0f + (rng() % 300);
    			if (rng() % 100 < AIRCRAFT_SPAWN_PROBABILITY * 100) {
    				auto aircraft = std::make_shared<Aircraft>(*aircraftTemplate);
    				aircraft->setPosition(glm::vec3(coord.x * CHUNK_SIZE + (rand() % CHUNK_SIZE), height, coord.z * CHUNK_SIZE + (rand() % CHUNK_SIZE)));
    				aircraft->setRotation(glm::vec3(0, rand() % 360, 0));
    				chunkEntities.push_back(aircraft);
    			}

    			// Robot with a 20% chance
    			glm::vec3 centerPosition(coord.x * CHUNK_SIZE + CHUNK_SIZE / 2, 0, coord.z * CHUNK_SIZE + CHUNK_SIZE / 2);
    			if (rng() % 10 < 2) {
    				auto bot = std::make_shared<MyBot>(*botTemplate);
    				bot->setPosition(glm::vec3(centerPosition.x, -33, centerPosition.z));
    				bot->setRotation(glm::vec3(0, rand() % 360, 0));
    				chunkEntities.push_back(bot);
    			}

    			glm::vec3 edgePositions[4];

    			const float inset = 10.0f;

    			edgePositions[0] = glm::vec3(coord.x * CHUNK_SIZE + CHUNK_SIZE / 2 + (rng() % 10 - 5), 0, coord.z * CHUNK_SIZE + inset);
    			edgePositions[1] = glm::vec3((coord.x + 1) * CHUNK_SIZE - inset, 0, coord.z * CHUNK_SIZE + CHUNK_SIZE / 2 + (rng() % 10 - 5));
    			edgePositions[2] = glm::vec3(coord.x * CHUNK_SIZE + CHUNK_SIZE / 2 + (rng() % 10 - 5), 0, (coord.z + 1) * CHUNK_SIZE - inset);
    			edgePositions[3] = glm::vec3(coord.x * CHUNK_SIZE + inset, 0, coord.z * CHUNK_SIZE + CHUNK_SIZE / 2 + (rng() % 10 - 5));


    			int edgeIndex = rng() % 4;
    			// Tree with a 20% chance
    			if (rng() % 10 < 2) {
    				auto tree = std::make_shared<Tree>(*treeTemplate);
    				tree->setScale(glm::vec3(20.0f, 20.0f + (rng() % 5), 20.0f));
    				tree->setPosition(edgePositions[edgeIndex]);
    				chunkEntities.push_back(tree);
    			}

    		} else {
    			auto ground = std::make_shared<Ground>(*groundTemplate);
    			ground->setPosition(glm::vec3(
					coord.x * CHUNK_SIZE + CHUNK_SIZE / 2,
					0,
					coord.z * CHUNK_SIZE + CHUNK_SIZE / 2
				));
    			chunkEntities.push_back(ground);

    			const int numCrystals = 5 + (rng() % 10);
    			const float centerArea = CHUNK_SIZE * 0.2f;
    			const float baseWidth = 32.0f;

    			std::vector<glm::vec2> crystalPositions;


    			glm::vec2 centerPos(
					coord.x * CHUNK_SIZE + CHUNK_SIZE / 2,
					coord.z * CHUNK_SIZE + CHUNK_SIZE / 2
				);
    			crystalPositions.push_back(centerPos);

    			float firstHeight = 64.0f + (rng() % 128);
    			float firstScaleX = 0.5f + (float)(rng() % 100) / 100.0f;
    			float firstScaleZ = 0.5f + (float)(rng() % 100) / 100.0f;

    			auto firstCrystal = std::make_shared<Building>(*buildingTemplates[0]);
    			firstCrystal->setScale(glm::vec3(baseWidth * firstScaleX, firstHeight, baseWidth * firstScaleZ));
    			firstCrystal->setPosition(glm::vec3(centerPos.x, firstHeight , centerPos.y));
    			firstCrystal->setRotation(glm::vec3(0, rng() % 360, 0));
    			chunkEntities.push_back(firstCrystal);

    			for (int i = 1; i < numCrystals; i++) {
    				float randomHeight = 64.0f + (rng() % 128);
    				float randomScaleX = 0.5f + (float)(rng() % 100) / 100.0f;
    				float randomScaleZ = 0.5f + (float)(rng() % 100) / 100.0f;
    				auto crystal = std::make_shared<Building>(*buildingTemplates[0]);

    				float angle = (float)(rng() % 360) * 3.14159f / 180.0f;
    				float distance = (float)(rng() % (int)(centerArea));

    				float offsetX = cos(angle) * distance;
    				float offsetZ = sin(angle) * distance;

    				glm::vec2 pos(
						centerPos.x + offsetX,
						centerPos.y + offsetZ
					);

    				glm::vec3 randomRotation(
						0,
						rng() % 360,
						0
					);

    				crystal->setScale(glm::vec3(baseWidth * randomScaleX, randomHeight, baseWidth * randomScaleZ));
    				crystal->setPosition(glm::vec3(pos.x, randomHeight, pos.y));
    				crystal->setRotation(randomRotation);
    				chunkEntities.push_back(crystal);
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

			// Render each entity in the chunk
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
				} else if (auto sphere = std::dynamic_pointer_cast<Sphere>(entity)) {
					sphere->render(vp, eye);
				}
			}
		}

	    if (!aircraftFound) {
			findInteractableAircraft(eye);
	    }

	}

	ChunkCoord getLightGroupCoord(const ChunkCoord& coord) {
		return {
			(coord.x / LIGHT_GROUP_SIZE) * LIGHT_GROUP_SIZE,
			(coord.z / LIGHT_GROUP_SIZE) * LIGHT_GROUP_SIZE
		};
	}

	void generateLightForGroup(const ChunkCoord& coord, std::vector<std::shared_ptr<Entity>>& chunkEntities) {
		ChunkCoord groupCoord = getLightGroupCoord(coord);

		// Check if light already exists for this group
		if (lightPoints.find(groupCoord) != lightPoints.end()) {
			return;
		}

		// Generate light for this group
		LightPoint light;
		light.position = glm::vec3(
			groupCoord.x * CHUNK_SIZE + (CHUNK_SIZE * LIGHT_GROUP_SIZE / 2),
			LIGHT_HEIGHT - 200,
			groupCoord.z * CHUNK_SIZE + (CHUNK_SIZE * LIGHT_GROUP_SIZE / 2)
		);
		light.color = glm::vec3(1.0, 0.95, 0.95);
		light.intensity = 1.0f;

		lightPoints[groupCoord] = light;

		// Add a sphere to represent the light
		auto sphere = std::make_shared<Sphere>(*sphereTemplate);
		sphere->setPosition(light.position);
		chunkEntities.push_back(sphere);
	}

	LightPoint getClosestLight(const ChunkCoord& coord) {
		return lightPoints[getLightGroupCoord(coord)];
	}


	std::shared_ptr<Aircraft> findInteractableAircraft(
		const glm::vec3& cameraPos
	) {
		bool found = false;
		const float INTERACTION_CUBE_SIZE = 200.0f;

		for (const auto& [_, chunk] : currentChunks) {
			for (auto& entity : chunk) {
				// Verify if the entity is an aircraft
				if (auto aircraft = std::dynamic_pointer_cast<Aircraft>(entity)) {
					// Change this condition to check for central aircraft instead
					if (aircraft->getIsCentral()) {  // <- Modified this line
						glm::vec3 aircraftPos = aircraft->getPosition();

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
		}

		// If no aircraft was found, disable interaction with the current one
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
		currentInteractableAircraft = findInteractableAircraft(
			cameraPos
		);

		// If an aircraft was found, interact with it
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
				std::abs(coord.z - currentChunk.z) > RENDER_DISTANCE + 1))
				{
				chunksToRemove.push_back(coord);
				lightPoints.erase(getLightGroupCoord(coord));
				}
		}

		for (const auto& coord : chunksToRemove) {
			currentChunks.erase(coord);
		}
	}

};



#endif