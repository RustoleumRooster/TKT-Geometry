#include "vkSystem4.h"
#include "vkDescriptors.h"
#include "vkComputePipeline.h"
#include "vkBufferObject.h"


void System4::cleanup()
{
	descriptorSetLayout->cleanup();
	pipeline->cleanup();

	if (shaderParamsBufferObject)
		delete shaderParamsBufferObject;

	vkDestroyPipelineLayout(device->getDevice(), pipelineLayout, nullptr);
}


VkSampler System4::createDefaultSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	samplerInfo.anisotropyEnable = VK_TRUE;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(device->getPhysicalDevice(), &properties);
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VkSampler sampler;

	if (vkCreateSampler(device->getDevice(), &samplerInfo, nullptr,
		&sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	return sampler;
}

void System4::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding paramsBufferBinding{};
	paramsBufferBinding.binding = 0;
	paramsBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	paramsBufferBinding.descriptorCount = 1;
	paramsBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	paramsBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkDescriptorSetLayoutBinding imageStorageBinding{};
	imageStorageBinding.binding = 2;
	imageStorageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	imageStorageBinding.descriptorCount = 1;
	imageStorageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	imageStorageBinding.pImmutableSamplers = nullptr; // Optional

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.resize(3);
	bindings = { paramsBufferBinding, samplerLayoutBinding, imageStorageBinding };

	descriptorSetLayout = new MyDescriptorSetLayout(device, bindings);
}

void System4::createDescriptorSets()
{
	MyDescriptorWriter writer(*descriptorSetLayout, *descriptorPool);

	descriptorSets.resize(1);

	VkDescriptorBufferInfo paramsInfo{};
	paramsInfo.buffer = shaderParamsBufferObject->getBuffer();
	paramsInfo.offset = 0;
	paramsInfo.range = sizeof(ShaderParamsInfo);

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageInfo.imageView = lightmapImageViewIn;
	//imageInfo.sampler = defaultSampler;

	VkDescriptorImageInfo imageStorageInfo{};
	imageStorageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageStorageInfo.imageView = lightmapImageViewOut;

	writer.writeBuffer(0, paramsInfo);
	writer.writeImage(1, imageInfo);
	writer.writeImage(2, imageStorageInfo);
	writer.build(descriptorSets[0]);
}

void System4::createComputePipeline()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &(descriptorSetLayout->getDescriptorSetLayout());

	if (vkCreatePipelineLayout(device->getDevice(), &pipelineLayoutInfo, nullptr,
		&pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create compute pipeline layout!");
	}

	pipeline = new ComputePipeline(device, "shaders/compute_lm_edges.spv", pipelineLayout);

}

void System4::setLightmapIn(VkImage img, VkDeviceMemory imgMem, VkImageView imgView, uint32_t size)
{
	lightmapImageIn = img;
	lightmapImageViewIn = imgView;
	lightmapMemoryIn = imgMem;
	lightmap_size = size;

	if (shaderParamsBufferObject)
		delete shaderParamsBufferObject;

	shaderParamsBufferObject = new MyBufferObject(device, sizeof(ShaderParamsInfo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 16);

	ShaderParamsInfo info;

	info.lightmap_height = size;
	info.lightmap_width = size;

	shaderParamsBufferObject->writeToBuffer((void*)&info);

}

void System4::setLightmapOut(VkImage img, VkDeviceMemory imgMem, VkImageView imgView)
{
	lightmapImageOut = img;
	lightmapImageViewOut = imgView;
	lightmapMemoryOut = imgMem;
}


void System4::execute()
{

	createDescriptorSets();

	std::cout << "executing compute shader\n";

	VkCommandBuffer commandBuffer = device->beginSingleTimeCommands();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline->getPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
		&descriptorSets[0], 0, 0);

	uint32_t work_length = 1;
	uint32_t work_height = 1;

	uint32_t n_WorkGroups_x = 1 + lightmap_size / (32 * work_length);
	uint32_t n_WorkGroups_y = 1 + lightmap_size / (8 * work_height);

	vkCmdDispatch(commandBuffer, n_WorkGroups_x, n_WorkGroups_y, 1);

	device->endSingleTimeCommands(commandBuffer);

	descriptorPool->freeDescriptorsSets(descriptorSets);

	vkDeviceWaitIdle(device->getDevice());
	std::cout << "...execution complete\n";

}
