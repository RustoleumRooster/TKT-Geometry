#pragma once

#ifndef _VULKAN_APP_H_
#define _VULKAN_APP_H_


#include <vulkan/vulkan.h>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <set>

#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp

#include <fstream>
#include <array>

//#include "SwapChain.h"
//#include "GraphicsPipeline.h"
#include "vkDevice.h"
#include "vkDescriptors.h"
#include "vkModel.h"
#include "vkTextures.h"
#include "vkComputePipeline.h"
#include "vkSystem5.h"
#include "vkSystem6.h"

class MeshNode_Interface_Final;

class VulkanApp {
public:
	~VulkanApp()
	{
		delete m_Textures;
		//delete m_DescriptorSetLayout;
		delete m_DescriptorPool;
		//delete m_GraphicsPipeline;
		delete m_device;
	}

	void run(MeshNode_Interface_Final* meshnode, LineHolder& graph) {
		initVulkan(meshnode);
		//mainLoop();

		system6->executeComputeShader();
		vkDeviceWaitIdle(m_device->getDevice());

		system6->writeDrawLines(graph);

		cleanup();
	}

private:

	void initVulkan(MeshNode_Interface_Final* meshnode) {
		m_device = new MyDevice();
		m_Textures = new MyTextures(m_device);

		createUniformBuffers();

		system6 = new System6(m_device, uniformBuffers);
		system6->loadModel(meshnode);

		createDescriptorPool();

		system6->initialize_step2(m_DescriptorPool);

		createCommandBuffers();
		//createSyncObjects();
	}

	void createDescriptorPool();

	void updateUniformBuffer(uint32_t currentImage);
	void createUniformBuffers();

	//void createSyncObjects();

	void createCommandBuffers();

	void mainLoop() {
		
	}

	void cleanup() {

		m_Textures->cleanup();

		/*
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(m_device->getDevice(), uniformBuffers[i], nullptr);
			vkFreeMemory(m_device->getDevice(), uniformBuffersMemory[i], nullptr);
		}*/

		m_DescriptorPool->cleanup();
		//system2->cleanup();
		system6->cleanup();


		m_device->cleanup();

	}


	uint32_t currentFrame = 0;

	MyDevice* m_device = NULL;

	MyDescriptorPool* m_DescriptorPool = NULL;

	MyTextures* m_Textures = NULL;

	System5* system5 = NULL;

	System6* system6 = NULL;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	std::vector<VkCommandBuffer> commandBuffers;

	//Vulkan_Model m_model;
};

#endif