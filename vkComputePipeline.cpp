
#include <vulkan/vulkan.h>

#include <iostream>
#include <fstream>

#include "vkComputePipeline.h"

std::vector<char> ComputePipeline::readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

void ComputePipeline::createComputePipeline(const char* shaderFilePath, VkPipelineLayout pipelineLayout)
{
	auto vertShaderCode = readFile(shaderFilePath);
	//std::cout << "loaded compute shader file: " << vertShaderCode.size() << " bytes\n";

	VkShaderModule shaderModule;
	createShaderModule(vertShaderCode, &shaderModule);

	VkPipelineShaderStageCreateInfo shaderStageInfo{};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderStageInfo.module = shaderModule;
	shaderStageInfo.pName = "main";

	VkComputePipelineCreateInfo pipelineInfo{};

	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.stage = shaderStageInfo;
	
	if (vkCreateComputePipelines(device->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create compute pipeline!");
	}
	
	vkDestroyShaderModule(device->getDevice(), shaderModule, nullptr);
}

void ComputePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* module) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(device->getDevice(), &createInfo, nullptr,
		module) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}
}
