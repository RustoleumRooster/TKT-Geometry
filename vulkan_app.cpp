#include "vulkan_app.h"

#include "vkModel.h"
#include "vkBufferObject.h"
#include "vkTextures.h"

 void VulkanApp::createDescriptorPool() {

	 std::vector<VkDescriptorPoolSize> poolSizes{};
	 poolSizes.resize(3);

	 poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	 poolSizes[0].descriptorCount = 1;


	 poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	 poolSizes[1].descriptorCount = 4;

	 poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	 poolSizes[2].descriptorCount = 1;

	 m_DescriptorPool = new MyDescriptorPool(m_device, 6, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, poolSizes);
 }

 void VulkanApp::updateUniformBuffer(uint32_t currentImage)
{
	 /*
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();

	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	glm::mat4 rot = glm::rotate(glm::mat4(1), time * glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::vec3 eye = rot*glm::vec4(500, 220, 25,0);

	UniformBufferObject ubo{};
	ubo.model = glm::mat4(1);//glm::rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(system1->getModelCenter() +eye
		, system1->getModelCenter(), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), m_swapChain.getExtent().width / (float)m_swapChain.getExtent().height, 10.f, 5000.0f);
	ubo.proj[1][1] *= -1;

	memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));*/
}

 void VulkanApp::createUniformBuffers() {
	 /*
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_device->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i],
			uniformBuffersMemory[i]);

		vkMapMemory(m_device->getDevice(), uniformBuffersMemory[i], 0, bufferSize,
			0, &uniformBuffersMapped[i]);
	}*/
}


 void VulkanApp::createCommandBuffers() {
	commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_device->getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(m_device->getDevice(), &allocInfo, commandBuffers.data()) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}



