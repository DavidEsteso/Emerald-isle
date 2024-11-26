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

#include <stb/stb_image.h>

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
#include <model1.h>
#include <model2.h>



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

    // Texture paths array
    const char* texturePaths[6] = {
        "../lab2/textures/facade0.jpg",
    	"..lab2/textures/facade1.jpg",
    	"..lab2/textures/facade2.jpg",
    	"..lab2/textures/facade3.jpg",
    	"..lab2/textures/facade4.jpg",
    	"..lab2/textures/facade5.jpg"


    };

	const char* groundTexturePaths[6] = {
		"../lab2/textures/facade0.jpg",
		"..lab2/textures/facade1.jpg",
		"..lab2/textures/facade2.jpg",
		"..lab2/textures/facade3.jpg",
		"..lab2/textures/facade4.jpg",
		"..lab2/textures/facade5.jpg"
	};

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

        // Existing chunk generation logic
        for (int i = 0; i < BUILDINGS_PER_ROW; ++i) {
            for (int j = 0; j < BUILDINGS_PER_ROW; ++j) {
                auto building = std::make_shared<Building>();

                // Generate consistent random values
                unsigned seed = std::hash<int>()(coord.x ^ (coord.z << 16) ^ (i << 8) ^ j);
                std::srand(seed);

                const char* texturePath = texturePaths[std::rand() % 6];
                float height = (std::rand() % 5 + 1) * 16.0f;
                float width = (std::rand() % 2) ? 32.0f : 16.0f;
                float depth = (std::rand() % 2) ? 32.0f : 16.0f;

                float baseX = coord.x * CHUNK_SIZE;
                float baseZ = coord.z * CHUNK_SIZE;

                float x = baseX + i * (BLOCK_SIZE + STREET_WIDTH);
                float z = baseZ + j * (BLOCK_SIZE + STREET_WIDTH);

                glm::vec3 position(
                    x + (BLOCK_SIZE - width) * 0.5f,
                    height * 0.5f + 50.0f,
                    z + (BLOCK_SIZE - depth) * 0.5f
                );

                building->initialize(position, glm::vec3(width, height, depth), "../lab2/textures/cube");
                chunkEntities.push_back(building);
            }
        }

        // Add ground
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
        glm::vec3 groundScale(CHUNK_SIZE * 0.5f, 0.0f, CHUNK_SIZE * 0.5f);

        ground->initialize(groundPosition, groundScale, groundTexture);
        chunkEntities.push_back(ground);

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

	void render(glm::mat4 vp, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, glm::vec3 eye) {
    	for (auto& chunk : chunks) {
    		for (auto& entity : chunk.second) {
    			// Polymorphic rendering
    			if (auto building = dynamic_cast<Building*>(entity.get())) {
    				building->render(vp, viewMatrix, projectionMatrix, eye);
    			} else if (auto ground = dynamic_cast<Ground*>(entity.get())) {
					ground->render(vp);
				}
    		}
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