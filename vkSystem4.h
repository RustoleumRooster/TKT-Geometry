#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <stdexcept>
#include "vkDevice.h"
#include "vkModel.h"

class MyDescriptorSetLayout;
class SwapChain;
class MyTextures;
class MyDescriptorPool;
class ComputePipeline;
class MyBufferObject;

class System4
{
	struct ShaderParamsInfo {
		uint32_t lightmap_width;
		uint32_t lightmap_height;
	};

public:

	System4(MyDevice* device) :
		device{ device }
	{
	}

	void setDescriptorPool(MyDescriptorPool* descriptorPool) {
		this->descriptorPool = descriptorPool;
	}

	void createDescriptorSets();
	void createDescriptorSetLayout();
	void createComputePipeline();
	void setLightmapIn(VkImage, VkDeviceMemory, VkImageView, uint32_t size);
	void setLightmapOut(VkImage, VkDeviceMemory, VkImageView);
	void execute();
	void cleanup();

	VkSampler createDefaultSampler();

private:
	VkPipelineLayout pipelineLayout;
	std::vector<VkDescriptorSet> descriptorSets;

	MyDevice* device = NULL;
	MyDescriptorPool* descriptorPool = NULL;

	MyDescriptorSetLayout* descriptorSetLayout = NULL;
	ComputePipeline* pipeline = NULL;
	MyBufferObject* shaderParamsBufferObject = NULL;

	VkImage lightmapImageIn, lightmapImageOut;
	VkDeviceMemory lightmapMemoryIn, lightmapMemoryOut;
	VkImageView lightmapImageViewIn, lightmapImageViewOut;
	uint32_t lightmap_size = 0;
};