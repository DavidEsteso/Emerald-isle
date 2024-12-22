//
// Created by david on 19/12/2024.
//

#ifndef BOT_H
#define BOT_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <math.h>
#include <GL/glext.h>
#include <tiny_gltf.h>
#include <render/shader.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include "entity.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))


struct MyBot : public Entity{
	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint jointMatricesID;
	GLuint lightPositionID;
	GLuint lightIntensityID;
	GLuint jointCountID;

	tinygltf::Model model;

	GLuint cubemapTextureID;

	// Each VAO corresponds to each mesh primitive in the GLTF model
	struct PrimitiveObject {
		GLuint vao;
		std::map<int, GLuint> vbos;
		bool isShoulder = false;
	};
	std::vector<PrimitiveObject> primitiveObjects;

	// Skinning
	struct SkinObject {
		// Transforms the geometry into the space of the respective joint
		std::vector<glm::mat4> inverseBindMatrices;

		// Transforms the geometry following the movement of the joints
		std::vector<glm::mat4> globalJointTransforms;

		// Combined transforms
		std::vector<glm::mat4> jointMatrices;
	};
	std::vector<SkinObject> skinObjects;

	// Animation
	struct SamplerObject {
		std::vector<float> input;
		std::vector<glm::vec4> output;
		int interpolation;
	};
	struct ChannelObject {
		int sampler;
		std::string targetPath;
		int targetNode;
	};
	struct AnimationObject {
		std::vector<SamplerObject> samplers;	// Animation data
	};
	std::vector<AnimationObject> animationObjects;

	GLuint diffuseTextureID;
	GLuint normalTextureID;
	GLuint armTextureID;



	// The current global transforms for each joint
	// Update this will result in skeleton in different poses

	glm::mat4 getNodeTransform(const tinygltf::Node& node) {
		glm::mat4 transform(1.0f);

		if (node.matrix.size() == 16) {
			transform = glm::make_mat4(node.matrix.data());
		} else {
			if (node.translation.size() == 3) {
				transform = glm::translate(transform, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
			}
			if (node.rotation.size() == 4) {
				glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
				transform *= glm::mat4_cast(q);
			}
			if (node.scale.size() == 3) {
				transform = glm::scale(transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
			}
		}
		return transform;
	}

	void computeLocalNodeTransform(const tinygltf::Model& model,
		int nodeIndex,
		std::vector<glm::mat4> &localTransforms)
	{
		// ---------------------------------------
		// TODO: your code here
		// ---------------------------------------
		const tinygltf::Node& node = model.nodes[nodeIndex];

		localTransforms[nodeIndex] = getNodeTransform(model.nodes[nodeIndex]);
		//recursively call the function for the children
		for (int childIndex : node.children) {
			computeLocalNodeTransform(model, childIndex, localTransforms);
		}

	}

	void computeGlobalNodeTransform(const tinygltf::Model& model,
		const std::vector<glm::mat4> &localTransforms,
		int nodeIndex, const glm::mat4& parentTransform,
		std::vector<glm::mat4> &globalTransforms)
	{
		// ----------------------------------------
		// TODO: your code here
		// ----------------------------------------
		// Get the node's local transformation matrix
		glm::mat4 localTransform = localTransforms[nodeIndex];

		// The global transformation is the parent transform multiplied by the node's local transform
		glm::mat4 globalTransform = parentTransform * localTransform;

		// Store the global transformation matrix for the node
		globalTransforms[nodeIndex] = globalTransform;

		//recursively compute the global transforms for the children
		const tinygltf::Node& node = model.nodes[nodeIndex];
		for (int childIndex : node.children) {
			computeGlobalNodeTransform(model, localTransforms, childIndex, globalTransform, globalTransforms);
		}

	}

	std::vector<SkinObject> prepareSkinning(const tinygltf::Model &model) {
		std::vector<SkinObject> skinObjects;


		for (const auto& skin : model.skins) {

			SkinObject skinObject;

			if (skin.inverseBindMatrices < 0 || skin.inverseBindMatrices >= model.accessors.size()) {
				continue;
			}

			const tinygltf::Accessor& accessor = model.accessors[skin.inverseBindMatrices];


			assert(accessor.type == TINYGLTF_TYPE_MAT4);
			const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
			const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
			const float *ptr = reinterpret_cast<const float *>(
				buffer.data.data() + accessor.byteOffset + bufferView.byteOffset);

			skinObject.inverseBindMatrices.resize(accessor.count);
			for (size_t j = 0; j < accessor.count; j++) {
				float m[16];
				memcpy(m, ptr + j * 16, 16 * sizeof(float));
				skinObject.inverseBindMatrices[j] = glm::make_mat4(m);
			}

			assert(skin.joints.size() == accessor.count);
			skinObject.globalJointTransforms.resize(skin.joints.size());
			skinObject.jointMatrices.resize(skin.joints.size());

			// Compute local and global transforms
			std::vector<glm::mat4> localTransforms(skin.joints.size());
			computeLocalNodeTransform(model, skin.joints[0], localTransforms);
			computeGlobalNodeTransform(model, localTransforms, skin.joints[0], localTransforms[skin.joints[0]], skinObject.globalJointTransforms);

			// Compute joint matrices
			for (size_t j = 0; j < skin.joints.size(); j++) {
				int jointIndex = skin.joints[j];
				skinObject.jointMatrices[j] = localTransforms[jointIndex] * skinObject.inverseBindMatrices[j];
			}

			skinObjects.push_back(skinObject);
		}
		return skinObjects;
	}


	int findKeyframeIndex(const std::vector<float>& times, float animationTime, bool enableDebug = false)
	{
	    // Debugging macro
	    #define DEBUG_PRINT(x) do { if(enableDebug) std::cout << x << std::endl; } while(0)

	    // Input validation
	    if (times.empty()) {
	        DEBUG_PRINT("Error: Empty times vector");
	        return 0;
	    }

	    // Handle edge cases
	    if (animationTime <= times.front()) {
	        DEBUG_PRINT("Animation time before first keyframe: " << animationTime << " <= " << times.front());
	        return 0;
	    }

	    if (animationTime >= times.back()) {
	        DEBUG_PRINT("Animation time after last keyframe: " << animationTime << " >= " << times.back());
	        return times.size() - 2;
	    }

	    int left = 0;
	    int right = times.size() - 1;

	    // Enhanced binary search with more robust error handling
	    while (left <= right) {
	        int mid = left + (right - left) / 2;  // Prevent potential integer overflow

	        DEBUG_PRINT("Iteration - Left: " << left << ", Mid: " << mid << ", Right: " << right);
	        DEBUG_PRINT("Current times - Mid: " << times[mid] << ", Mid+1: " << times[mid+1]);
	        DEBUG_PRINT("Animation Time: " << animationTime);

	        // Check if animation time is between current and next keyframe
	        if (mid + 1 < times.size() &&
	            times[mid] <= animationTime &&
	            animationTime < times[mid + 1]) {

	            DEBUG_PRINT("Found keyframe index: " << mid);
	            return mid;
	        }

	        // Adjust search range
	        if (times[mid] > animationTime) {
	            right = mid - 1;
	            DEBUG_PRINT("Moving left - New right: " << right);
	        }
	        else {
	            left = mid + 1;
	            DEBUG_PRINT("Moving right - New left: " << left);
	        }

	        // Prevent infinite loop (though unlikely with binary search)
	        if (left > right) {
	            DEBUG_PRINT("Warning: Keyframe search exhausted");
	            break;
	        }
	    }

	    // Fallback to last two keyframes if no exact match found
	    DEBUG_PRINT("Fallback to last two keyframes");
	    return times.size() - 2;

	    #undef DEBUG_PRINT
	}

	std::vector<AnimationObject> prepareAnimation(const tinygltf::Model &model)
	{
		std::vector<AnimationObject> animationObjects;
		for (const auto &anim : model.animations) {
			AnimationObject animationObject;

			for (const auto &sampler : anim.samplers) {
				SamplerObject samplerObject;

				const tinygltf::Accessor &inputAccessor = model.accessors[sampler.input];
				const tinygltf::BufferView &inputBufferView = model.bufferViews[inputAccessor.bufferView];
				const tinygltf::Buffer &inputBuffer = model.buffers[inputBufferView.buffer];

				assert(inputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
				assert(inputAccessor.type == TINYGLTF_TYPE_SCALAR);

				// Input (time) values
				samplerObject.input.resize(inputAccessor.count);

				const unsigned char *inputPtr = &inputBuffer.data[inputBufferView.byteOffset + inputAccessor.byteOffset];
				const float *inputBuf = reinterpret_cast<const float*>(inputPtr);

				// Read input (time) values
				int stride = inputAccessor.ByteStride(inputBufferView);
				for (size_t i = 0; i < inputAccessor.count; ++i) {
					samplerObject.input[i] = *reinterpret_cast<const float*>(inputPtr + i * stride);
				}

				const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
				const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
				const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

				assert(outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
				const float *outputBuf = reinterpret_cast<const float*>(outputPtr);

				int outputStride = outputAccessor.ByteStride(outputBufferView);

				// Output values
				samplerObject.output.resize(outputAccessor.count);

				for (size_t i = 0; i < outputAccessor.count; ++i) {

					if (outputAccessor.type == TINYGLTF_TYPE_VEC3) {
						memcpy(&samplerObject.output[i], outputPtr + i * 3 * sizeof(float), 3 * sizeof(float));
					} else if (outputAccessor.type == TINYGLTF_TYPE_VEC4) {
						memcpy(&samplerObject.output[i], outputPtr + i * 4 * sizeof(float), 4 * sizeof(float));
					} else {
						std::cout << "Unsupport accessor type ..." << std::endl;
					}

				}

				animationObject.samplers.push_back(samplerObject);
			}

			animationObjects.push_back(animationObject);
		}
		return animationObjects;
	}

	void updateAnimation(
	    const tinygltf::Model &model,
	    const tinygltf::Animation &anim,
	    const AnimationObject &animationObject,
	    float time,
	    std::vector<glm::mat4> &nodeTransforms)
	{
		for (const auto &channel : anim.channels) {
			int targetNodeIndex = channel.target_node;
			const auto &sampler = anim.samplers[channel.sampler];

			// Access output (value) data for the channel
			const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
			const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
			const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

			// Calculate current animation time (wrap if necessary)
			const std::vector<float> &times = animationObject.samplers[channel.sampler].input;
			float animationTime = fmod(time, times.back());

			// Find keyframe index
			int keyframeIndex = findKeyframeIndex(times, animationTime);

			const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
			const float *outputBuf = reinterpret_cast<const float*>(outputPtr);

			// Handle smooth looping
			int nextKeyframeIndex = (keyframeIndex + 1) % times.size();
			float t = (animationTime - times[keyframeIndex]) / (times[nextKeyframeIndex] - times[keyframeIndex]);

			if (channel.target_path == "translation") {
				glm::vec3 translation0, translation1;
				memcpy(&translation0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
				memcpy(&translation1, outputPtr + nextKeyframeIndex * 3 * sizeof(float), 3 * sizeof(float));

				// Smooth interpolation with potential wrapping
				glm::vec3 translation = glm::mix(translation0, translation1, t);
				nodeTransforms[targetNodeIndex] = glm::translate(nodeTransforms[targetNodeIndex], translation);
			}
			else if (channel.target_path == "rotation") {
				glm::quat rotation0, rotation1;
				memcpy(&rotation0, outputPtr + keyframeIndex * 4 * sizeof(float), 4 * sizeof(float));
				memcpy(&rotation1, outputPtr + nextKeyframeIndex * 4 * sizeof(float), 4 * sizeof(float));

				// Spherical linear interpolation with potential wrapping
				glm::quat rotation = glm::slerp(rotation0, rotation1, t);
				nodeTransforms[targetNodeIndex] *= glm::mat4_cast(rotation);
			}
			else if (channel.target_path == "scale") {
				glm::vec3 scale0, scale1;
				memcpy(&scale0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
				memcpy(&scale1, outputPtr + nextKeyframeIndex * 3 * sizeof(float), 3 * sizeof(float));

				// Smooth interpolation with potential wrapping
				glm::vec3 scale = glm::mix(scale0, scale1, t);
				nodeTransforms[targetNodeIndex] = glm::scale(nodeTransforms[targetNodeIndex], scale);
			}
		}
	}

	void updateSkinning(const std::vector<glm::mat4> &nodeTransforms) {

		for (size_t i = 0; i < skinObjects.size(); i++) {
			SkinObject &skinObject = skinObjects[i];
			for (size_t j = 0; j < skinObject.jointMatrices.size(); j++)
			{
				int jointIndex = model.skins[i].joints[j];
				skinObject.jointMatrices[j] = nodeTransforms[jointIndex] * skinObject.inverseBindMatrices[j];
			}
		}
	}

	void update(float time) {

		if (model.animations.size() > 0) {
			const tinygltf::Animation &animation = model.animations[0];
			const AnimationObject &animationObject = animationObjects[0];

			const tinygltf::Skin &skin = model.skins[0];
			std::vector<glm::mat4> nodeTransforms(skin.joints.size());

			for (size_t i = 0; i < nodeTransforms.size(); ++i) {
				nodeTransforms[i] = glm::mat4(1.0);
			}

			updateAnimation(model, animation, animationObject, time, nodeTransforms);

			int rootNodeIndex = -1;
			for (size_t i = 0; i < model.nodes.size(); ++i) {
				if (model.nodes[i].name == "root" || model.nodes[i].name == "Hips") {
					rootNodeIndex = i;
					break;
				}
			}

			std::vector<glm::mat4> globalNodeTransforms(skin.joints.size());
			computeGlobalNodeTransform(model, nodeTransforms, rootNodeIndex, glm::mat4(1.0f), globalNodeTransforms);

			updateSkinning(globalNodeTransforms);
		}
	}

	bool loadModel(tinygltf::Model &model, const char *filename) {
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
		if (!warn.empty()) {
			std::cout << "WARN: " << warn << std::endl;
		}

		if (!err.empty()) {
			std::cout << "ERR: " << err << std::endl;
		}

		if (!res)
			std::cout << "Failed to load glTF: " << filename << std::endl;
		else
			std::cout << "Loaded glTF: " << filename << std::endl;

		return res;
	}

	void initialize(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation) {
		this->position = position;
		this->scale = scale;
		this->rotation = rotation;

		updateModelMatrix();
		// Modify your path if needed
		if (!loadModel(model, "../lab2/models/bot/untitled.gltf")) {
			std::cout << "Failed to load model" << std::endl;
			return;
		}


		// Prepare buffers for rendering
		primitiveObjects = bindModel(model);

		std::cout << "model binded" << std::endl;

		// Prepare joint matrices
		skinObjects = prepareSkinning(model);

		std::cout << "skin prepared" << std::endl;

		// Prepare animation data
		animationObjects = prepareAnimation(model);

		std::cout << "animation prepared" << std::endl;

		// Create and compile our GLSL program from the shaders
		programID = LoadShadersFromFile("../lab2/shaders/bot.vert", "../lab2/shaders/bot.frag");
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		initLightUniforms();


		// Get a handle for GLSL variables
		mvpMatrixID = glGetUniformLocation(programID, "MVP");
		lightPositionID = glGetUniformLocation(programID, "lightPosition");
		lightIntensityID = glGetUniformLocation(programID, "lightIntensity");

		jointMatricesID = glGetUniformLocation(programID, "jointMatrices");  // For joint matrices

		cubemapTextureID = LoadCubeMap("../lab2/textures/cube_");



		/**
		// Just take the first skin/skeleton model
		const tinygltf::Skin &skin = model.skins[0];

		// Compute local transforms at each node
		int rootNodeIndex = skin.joints[0];
		std::vector<glm::mat4> localNodeTransforms(skin.joints.size());
		computeLocalNodeTransform(model, rootNodeIndex, localNodeTransforms);

		// Compute global transforms at each node
		glm::mat4 parentTransform(1.0f);
		std::vector<glm::mat4> globalNodeTransforms(skin.joints.size());
		computeGlobalNodeTransform(model, localNodeTransforms, rootNodeIndex, parentTransform, globalNodeTransforms);

		globalTransforms = globalNodeTransforms;
		**/

		std::cout << "Initialized" << std::endl;
	}

	void render(glm::mat4 cameraMatrix) {

		glUseProgram(programID);

		updateModelMatrix();

		glm::mat4 mvp = cameraMatrix * modelMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

		if (!skinObjects.empty()) {
			glUniformMatrix4fv(jointMatricesID,
							  static_cast<GLsizei>(skinObjects[0].jointMatrices.size()),
							  GL_FALSE,
							  glm::value_ptr(skinObjects[0].jointMatrices[0]));
		}

		GLuint isShoulderLocation = glGetUniformLocation(programID, "isShoulderUniform");

		for (size_t i = 0; i < primitiveObjects.size(); ++i) {
			glBindVertexArray(primitiveObjects[i].vao);
			glUniform1i(isShoulderLocation, primitiveObjects[i].isShoulder ? 1 : 0);
		}

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);
		GLuint cubemapSamplerID = glGetUniformLocation(programID, "cubemapSampler");
		glUniform1i(cubemapSamplerID, 3);

		GLuint refractionIndexID = glGetUniformLocation(programID, "refractionIndex");
		glUniform1f(refractionIndexID, 1.52f);

		drawModel(primitiveObjects, model);



	}

void bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
              tinygltf::Model &model,
              tinygltf::Mesh &mesh) {
    std::cout << "Binding Mesh: " << mesh.name << std::endl;

    std::map<int, GLuint> vbos;
    for (size_t i = 0; i < model.bufferViews.size(); ++i) {
        const tinygltf::BufferView &bufferView = model.bufferViews[i];

        int target = bufferView.target;

        if (bufferView.target == 0) {
            continue;
        }

        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(target, vbo);
        glBufferData(target, bufferView.byteLength,
                    &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

        vbos[i] = vbo;
    }

    // Each mesh can contain several primitives (or parts)
    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        tinygltf::Primitive primitive = mesh.primitives[i];
        tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        for (auto &attrib : primitive.attributes) {
            tinygltf::Accessor accessor = model.accessors[attrib.second];
            int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
            glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

            int size = 1;
            if (accessor.type != TINYGLTF_TYPE_SCALAR) {
                size = accessor.type;
            }

            int vaa = -1;
            if (attrib.first.compare("POSITION") == 0) vaa = 0;
            if (attrib.first.compare("NORMAL") == 0) vaa = 1;
            if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;
            if (attrib.first.compare("JOINTS_0") == 0) vaa = 3;
            if (attrib.first.compare("WEIGHTS_0") == 0) vaa = 4;
            if (vaa > -1) {
                glEnableVertexAttribArray(vaa);
                glVertexAttribPointer(vaa, size, accessor.componentType,
                                    accessor.normalized ? GL_TRUE : GL_FALSE,
                                    byteStride, BUFFER_OFFSET(accessor.byteOffset));
            } else {
                std::cout << "vaa missing: " << attrib.first << std::endl;
            }
        }

        // Record VAO for later use
        PrimitiveObject primitiveObject;
        primitiveObject.vao = vao;
        primitiveObject.vbos = vbos;


        primitiveObjects.push_back(primitiveObject);

        glBindVertexArray(0);
    }
}

void bindModelNodes(std::vector<PrimitiveObject> &primitiveObjects,
                    tinygltf::Model &model,
                    tinygltf::Node &node,
                    bool parentIsShoulder = false)
{
    bool isShoulder = parentIsShoulder;

    // Check for shoulder/arm in node name
    if (!node.name.empty()) {
        std::string lowercaseName;
        std::transform(node.name.begin(), node.name.end(), std::back_inserter(lowercaseName), ::tolower);

        bool nameIndicatesShoulder = (lowercaseName.find("shoulder") != std::string::npos) ||
                                     (lowercaseName.find("arm") != std::string::npos);

        isShoulder |= nameIndicatesShoulder;
    }

    // Recursively search for meshes in children
    bool foundMeshInChildren = false;
    for (int childIndex : node.children) {
        assert((childIndex >= 0) && (childIndex < model.nodes.size()));
        tinygltf::Node& childNode = model.nodes[childIndex];

        if ((childNode.mesh >= 0) && (childNode.mesh < model.meshes.size())) {


            size_t startPrimitiveCount = primitiveObjects.size();
            bindMesh(primitiveObjects, model, model.meshes[childNode.mesh]);

            // Mark primitives as shoulder
            for (size_t i = startPrimitiveCount; i < primitiveObjects.size(); ++i) {
                primitiveObjects[i].isShoulder = isShoulder;

            }

            foundMeshInChildren = true;
        }
    }

    // If no mesh found in children, check current node
    if (!foundMeshInChildren && (node.mesh >= 0) && (node.mesh < model.meshes.size())) {


        size_t startPrimitiveCount = primitiveObjects.size();
        bindMesh(primitiveObjects, model, model.meshes[node.mesh]);

        // Mark primitives as shoulder
        for (size_t i = startPrimitiveCount; i < primitiveObjects.size(); ++i) {
            primitiveObjects[i].isShoulder = isShoulder;

        }
    }

    // Recursively process children
    for (int childIndex : node.children) {
        assert((childIndex >= 0) && (childIndex < model.nodes.size()));
        bindModelNodes(primitiveObjects, model, model.nodes[childIndex], isShoulder);
    }
}


	std::vector<PrimitiveObject> bindModel(tinygltf::Model &model) {
		std::vector<PrimitiveObject> primitiveObjects;

		const tinygltf::Scene &scene = model.scenes[model.defaultScene];
		for (size_t i = 0; i < scene.nodes.size(); ++i) {
			assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
			bindModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]], false);  // Start with false
		}

		return primitiveObjects;
	}

	void drawMesh(const std::vector<PrimitiveObject> &primitiveObjects,
				tinygltf::Model &model, tinygltf::Mesh &mesh) {


		for (size_t i = 0; i < mesh.primitives.size(); ++i)
		{
			GLuint vao = primitiveObjects[i].vao;
			std::map<int, GLuint> vbos = primitiveObjects[i].vbos;

			glBindVertexArray(vao);

			tinygltf::Primitive primitive = mesh.primitives[i];
			tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));

			glDrawElements(primitive.mode, indexAccessor.count,
						indexAccessor.componentType,
						BUFFER_OFFSET(indexAccessor.byteOffset));

			glBindVertexArray(0);

		}
	}

	void drawModelNodes(const std::vector<PrimitiveObject>& primitiveObjects,
						tinygltf::Model &model, tinygltf::Node &node) {
		// Draw the mesh at the node, and recursively do so for children nodes
		if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
			drawMesh(primitiveObjects, model, model.meshes[node.mesh]);
		}
		for (size_t i = 0; i < node.children.size(); i++) {
			drawModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
		}
	}
	void drawModel(const std::vector<PrimitiveObject>& primitiveObjects,
				tinygltf::Model &model) {
		// Draw all nodes
		const tinygltf::Scene &scene = model.scenes[model.defaultScene];
		for (size_t i = 0; i < scene.nodes.size(); ++i) {
			drawModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
		}
	}




	void renderForShadows(const glm::mat4& lightSpaceMatrix, GLuint shadowProgramID) override {
		glUseProgram(shadowProgramID);

		// Set light space matrix uniform
		GLuint lightSpaceMatrixID = glGetUniformLocation(shadowProgramID, "lightSpaceMatrix");
		glUniformMatrix4fv(lightSpaceMatrixID, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

		// Set model matrix uniform
		GLuint modelMatrixID = glGetUniformLocation(shadowProgramID, "model");
		glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);

		// Draw model for shadow mapping
		drawModel(primitiveObjects, model);
	}


    void cleanup() override {
		glDeleteProgram(programID);
	}


};



#endif //BOT_H
