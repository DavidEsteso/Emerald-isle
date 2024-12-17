#ifndef CITY_H
#define CITY_H


#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

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

// Hash function structure for ChunkCoord
struct ChunkCoordHash {
    std::size_t operator()(const ChunkCoord& coord) const {
        return std::hash<int>()(coord.x) ^ (std::hash<int>()(coord.z) << 1);
    }
};

struct InfiniteCity {
    // Constants
    const int CHUNK_SIZE = 192;        // Size of chunk (6 buildings * (32*2 + 6))
    const int RENDER_DISTANCE = 1;      // How many chunks to render in each direction
    const float STREET_WIDTH = 6.0f;    // Width of streets between buildings
    const int BUILDINGS_PER_ROW = 1;    // Number of buildings per row in a chunk
	const float AIRCRAFT_SPAWN_PROBABILITY = 0.1f; // 50% chance of spawning an aircraft


	GLuint shadowMapFBO;
	GLuint shadowMapTexture;
	GLuint shadowProgramID;
	glm::mat4 lightSpaceMatrix;

	glm::vec3 lightPosition = glm::vec3(300.0f, 200.0f, 300.0f);
	glm::vec3 lightLookAt = glm::vec3(0.0f, -1.0f, 0.0f);

	bool saveDepth = true;



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

    ChunkCoord getChunkCoord(float x, float z) {
        return {
            static_cast<int>(std::floor(x / CHUNK_SIZE)),
            static_cast<int>(std::floor(z / CHUNK_SIZE))
        };
    }

    void generateChunk(const ChunkCoord& coord) {
        if (chunks.find(coord) != chunks.end()) return;

        const float BLOCK_SIZE = 32.0f;

        // Calculate base coordinates for this chunk
        float baseX = coord.x * CHUNK_SIZE;
        float baseZ = coord.z * CHUNK_SIZE;

        std::vector<std::shared_ptr<Entity>> chunkEntities;

        // Ground generation with larger size
        auto ground = std::make_shared<Ground>();
        unsigned seed = std::hash<int>()(coord.x ^ (coord.z << 16));
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> dist(0, 3);
        const char* groundTexture = groundTexturePaths[dist(rng)];

        glm::vec3 groundPosition(
            baseX + CHUNK_SIZE * 0.5f,
            0.0f,
            baseZ + CHUNK_SIZE * 0.5f
        );

        // Increase ground size
        glm::vec3 groundScale(CHUNK_SIZE, 0.0f, CHUNK_SIZE);
        ground->initialize(groundPosition, groundScale, groundTexture);
        chunkEntities.push_back(ground);

        // Calculate the center of the ground for building placement
        float centerX = baseX + CHUNK_SIZE * 0.5f;
        float centerZ = baseZ + CHUNK_SIZE * 0.5f;

        // Building generation
        float buildingAreaWidth = CHUNK_SIZE * 0.6f; // Use 60% of chunk width for buildings
        float startX = centerX - buildingAreaWidth * 0.5f;
        float startZ = centerZ - buildingAreaWidth * 0.5f;

        for (int i = 0; i < BUILDINGS_PER_ROW; ++i) {
            for (int j = 0; j < BUILDINGS_PER_ROW; ++j) {
                auto building = std::make_shared<Building>();

                // Use a consistent random seed
                unsigned buildingSeed = std::hash<int>()(coord.x ^ (coord.z << 16) ^ (i << 8) ^ j);
                std::srand(buildingSeed);

                const char* texturePath = texturePaths[std::rand() % 6];
                float height = (std::rand() % 5 + 1) * 16.0f;
                float width = (std::rand() % 2) ? 32.0f : 16.0f;
                float depth = (std::rand() % 2) ? 32.0f : 16.0f;

                float x = startX + i * (width + STREET_WIDTH);
                float z = startZ + j * (depth + STREET_WIDTH);

                glm::vec3 position(
                    x + (BLOCK_SIZE - width) * 0.5f,
                    height * 0.5f + 50.0f,
                    z + (BLOCK_SIZE - depth) * 0.5f
                );

                building->initialize(position, glm::vec3(width, height, depth), "../lab2/textures/cube");
                chunkEntities.push_back(building);
            }
        }

        // Aircraft generation (20% chance)
        std::uniform_real_distribution<float> probDist(0.0f, 1.0f);
        if (probDist(rng) < AIRCRAFT_SPAWN_PROBABILITY) {
            auto aircraft = std::make_shared<Aircraft>();

            // Position aircraft high above the ground
            float aircraftHeight = 150.0f + (std::rand() % 100); // Between 150 and 250
            glm::vec3 aircraftPosition(
                centerX + (std::rand() % 50 - 25), // Random offset from center
                aircraftHeight,
                centerZ + (std::rand() % 50 - 25)  // Random offset from center
            );

            aircraft->initialize(aircraftPosition, glm::vec3(20.0f, 20.0f, 20.0f));
            chunkEntities.push_back(aircraft);
        }

        // Store chunk entities
        chunks[coord] = chunkEntities;
    }


	void update(const glm::vec3& cameraPos) {
    	// Get current chunk from camera position
    	ChunkCoord currentChunk = getChunkCoord(cameraPos.x, cameraPos.z);

    	// Generate chunks within render distance
    	for (int dx = -RENDER_DISTANCE; dx <= RENDER_DISTANCE; dx++) {
    		for (int dz = -RENDER_DISTANCE; dz <= RENDER_DISTANCE; dz++) {
    			ChunkCoord newChunk = {currentChunk.x + dx, currentChunk.z + dz};
    			generateChunk(newChunk);
    		}
    	}

    	// Update all entities in chunks
    	for (auto& chunk : chunks) {
    		for (auto& entity : chunk.second) {
    			// Dynamic cast to handle different entity types
    			if (auto aircraft = dynamic_cast<Aircraft*>(entity.get())) {
    				// Add delta time calculation or pass it as a parameter
    				static auto lastTime = std::chrono::high_resolution_clock::now();
    				auto currentTime = std::chrono::high_resolution_clock::now();
    				float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
    				lastTime = currentTime;

    				aircraft->update(deltaTime);
    			}
    		}
    	}

    	// Clean up chunks outside render distance
    	std::vector<ChunkCoord> chunksToRemove;
    	for (const auto& chunk : chunks) {
    		if (std::abs(chunk.first.x - currentChunk.x) > RENDER_DISTANCE + 1 ||
				std::abs(chunk.first.z - currentChunk.z) > RENDER_DISTANCE + 1) {
    			chunksToRemove.push_back(chunk.first);
				}
    	}

    	// Clean up and remove chunks that are out of range
    	for (const auto& coord : chunksToRemove) {
    		// Clean up all entities in the chunk
    		for (auto& entity : chunks[coord]) {
    			entity->cleanup();
    		}

    		// Remove chunk from map
    		chunks.erase(coord);
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

	void render(glm::mat4 vp, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, glm::vec3 eye) {
    	// Render shadow map first
    	//renderShadowMap(lightPosition, lightLookAt);

    	// Render all entities in chunks
    	for (auto& chunk : chunks) {
    		for (auto& entity : chunk.second) {
    			if (auto building = dynamic_cast<Building*>(entity.get())) {
    				// Render buildings with shadow map
    				building->render(vp, viewMatrix, projectionMatrix, eye, shadowMapTexture, lightSpaceMatrix);
    			} else if (auto ground = dynamic_cast<Ground*>(entity.get())) {
    				// Render ground with shadow map
    				ground->render(vp, shadowMapTexture, lightSpaceMatrix);
    			} else if (auto aircraft = dynamic_cast<Aircraft*>(entity.get())) {
    				// Render aircraft
    				aircraft->render(vp, eye);
    			}
    		}
    	}

    	// Save depth texture if enabled
    	if (saveDepth) {
    		std::string filename = "depth_camera.png";
    		saveDepthTexture(shadowMapFBO, filename);
    		std::cout << "Depth texture saved to " << filename << std::endl;
    		saveDepth = false;
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
};

#endif