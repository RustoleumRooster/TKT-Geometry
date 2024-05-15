#pragma once

#include <vulkan/vulkan.h>

#include "vkDevice.h"

#include <vector>

class ComputePipeline
{
public:
	ComputePipeline(MyDevice* device_, const char* shaderFilePath, VkPipelineLayout pipelineLayout)
		: device(device_) {
		createComputePipeline(shaderFilePath, pipelineLayout);
	}

	~ComputePipeline() {
		//vkDestroyPipeline(device->getDevice(), computePipeline, nullptr);
	}

	VkPipeline getPipeline() {
		return computePipeline;
	}

	void cleanup()
	{
		vkDestroyPipeline(device->getDevice(), computePipeline, nullptr);
	}

private:

	static std::vector<char> readFile(const std::string& filename);

	void createComputePipeline(const char* shaderFilePath, VkPipelineLayout pipelineLayout);

	void createShaderModule(const std::vector<char>&, VkShaderModule* module);

	VkPipeline computePipeline;

	MyDevice* device;

};