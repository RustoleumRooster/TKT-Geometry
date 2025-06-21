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
#include "vkCompute_PointLight.h"
#include "vkCompute_TestMC.h"
#include "vkCompute_SoftLight.h"
#include "vkCompute_Light_Multipass.h"


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

	void run5(MeshNode_Interface_Final* meshnode, LineHolder& graph) {
		initVulkan();
		//mainLoop();

		system5 = new System5(m_device, uniformBuffers);
		system5->loadModel(meshnode);
		system5->initialize_step2(m_DescriptorPool);
		system5->executeComputeShader();
		vkDeviceWaitIdle(m_device->getDevice());

		//system5->writeDrawLines(graph);

		cleanup();
	}

	bool run_point_light(geometry_scene* geo_scene);
	bool run_soft_light(geometry_scene* geo_scene);
	bool run_multipass_light(geometry_scene* geo_scene);
	bool run_test_mc(geometry_scene* geo_scene);
	bool run_amb_occlusion(geometry_scene* geo_scene, std::string filename_base);


private:

	void initVulkan() {
		m_device = new MyDevice();
		m_Textures = new MyTextures(m_device);

		createUniformBuffers();
		createDescriptorPool();
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

		if (system5)
			system5->cleanup();

		//if(system6)
		//	system6->cleanup();

		if (system_point_light)
		{
			system_point_light->cleanup();
			delete system_point_light;
			system_point_light = NULL;
		}

		if (system_soft_light)
		{
			system_soft_light->cleanup();
			delete system_soft_light;
			system_soft_light = NULL;
		}

		if (system_multipass_light)
		{
			system_multipass_light->cleanup();
			delete system_multipass_light;
			system_multipass_light = NULL;
		}

		if (system_test_mc)
		{
			system_test_mc->cleanup();
			delete system_test_mc;
			system_test_mc = NULL;
		}

		if (system_amb_occlusion)
		{
			system_amb_occlusion->cleanup();
			delete system_amb_occlusion;
			system_amb_occlusion = NULL;
		}


		m_device->cleanup();

	}


	uint32_t currentFrame = 0;

	MyDevice* m_device = NULL;

	MyDescriptorPool* m_DescriptorPool = NULL;

	MyTextures* m_Textures = NULL;

	System5* system5 = NULL;


	System_Point_Light* system_point_light = NULL;
	System_Test_MC* system_test_mc = NULL;
	System_Amb_Occlusion* system_amb_occlusion = NULL;
	System_Soft_Light* system_soft_light = NULL;
	System_Light_Multipass* system_multipass_light = NULL;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	std::vector<VkCommandBuffer> commandBuffers;

	//Vulkan_Model m_model;
};

#endif