#include "vulkan_app.h"

#include "vkModel.h"
#include "vkBufferObject.h"
#include "vkTextures.h"
#include "geometry_scene.h"

bool  VulkanApp::run_point_light(geometry_scene* geo_scene) {

	initVulkan();

	system_point_light = new System_Point_Light(m_device, uniformBuffers);
	system_point_light->loadModel(&geo_scene->geoNode()->final_meshnode_interface);
	system_point_light->loadLights(geo_scene);
	system_point_light->initialize_step2(m_DescriptorPool);
	system_point_light->executeComputeShader();
	vkDeviceWaitIdle(m_device->getDevice());

	system_point_light->writeDrawLines(geo_scene->special_graph);

	cleanup();

	return true;
}


bool VulkanApp::run_test_mc(geometry_scene* geo_scene) {

	initVulkan();

	system_test_mc = new System_Test_MC(m_device, uniformBuffers);
	system_test_mc->loadModel(&geo_scene->geoNode()->final_meshnode_interface);
	system_test_mc->loadLights(geo_scene);
	system_test_mc->initialize_step2(m_DescriptorPool);
	system_test_mc->executeComputeShader();
	vkDeviceWaitIdle(m_device->getDevice());

	system_test_mc->writeDrawLines(geo_scene->special_graph);

	cleanup();

	return true;
}

bool VulkanApp::run_soft_light(geometry_scene* geo_scene) {

	initVulkan();

	system_soft_light = new System_Soft_Light(m_device, uniformBuffers);
	system_soft_light->loadModel(&geo_scene->geoNode()->final_meshnode_interface);
	system_soft_light->loadLights(geo_scene);
	system_soft_light->initialize_step2(m_DescriptorPool);
	system_soft_light->executeComputeShader();
	vkDeviceWaitIdle(m_device->getDevice());

	system_soft_light->writeDrawLines(geo_scene->special_graph);

	cleanup();

	return true;
}


bool VulkanApp::run_multipass_light(geometry_scene* geo_scene) {

	initVulkan();

	system_multipass_light = new System_Light_Multipass(m_device, uniformBuffers);
	system_multipass_light->loadModel(&geo_scene->geoNode()->final_meshnode_interface);
	system_multipass_light->loadLights(geo_scene);
	system_multipass_light->initialize_step2(m_DescriptorPool);
	system_multipass_light->executeComputeShader();
	vkDeviceWaitIdle(m_device->getDevice());

	system_multipass_light->writeDrawLines(geo_scene->special_graph);

	cleanup();

	return true;
}

bool VulkanApp::run_amb_occlusion(geometry_scene* geo_scene, std::string filename_base)
{
	initVulkan();

	system_amb_occlusion = new System_Amb_Occlusion(m_device, uniformBuffers);
	system_amb_occlusion->loadModel(&geo_scene->geoNode()->final_meshnode_interface);
	system_amb_occlusion->initialize_step2(m_DescriptorPool);
	system_amb_occlusion->executeComputeShader(filename_base);
	vkDeviceWaitIdle(m_device->getDevice());

	system_amb_occlusion->writeDrawLines(geo_scene->special_graph);

	cleanup();

	return true;
}

bool VulkanApp::run_sunlight(geometry_scene* geo_scene, std::string filename_base)
{
	initVulkan();

	system_sunlight = new System_Sunlight(m_device, uniformBuffers);
	system_sunlight->loadModel(&geo_scene->geoNode()->final_meshnode_interface);
	system_sunlight->loadSun(geo_scene);
	system_sunlight->initialize_step2(m_DescriptorPool);
	system_sunlight->executeComputeShader(filename_base);
	vkDeviceWaitIdle(m_device->getDevice());

	system_sunlight->writeDrawLines(geo_scene->special_graph);

	cleanup();

	return true;
}

 void VulkanApp::createDescriptorPool() {

	 std::vector<VkDescriptorPoolSize> poolSizes{};
	 poolSizes.resize(3);

	 poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	 poolSizes[0].descriptorCount = 1;


	 poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	 poolSizes[1].descriptorCount = 5;

	 poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	 poolSizes[2].descriptorCount = 2;

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



