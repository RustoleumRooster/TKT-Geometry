#pragma once

#include <vulkan/vulkan.h>
#include <irrlicht.h>

#include <vector>
#include <stdexcept>


using namespace irr;
using namespace core;

struct UniformBufferObject {
	alignas(16) matrix4 model;
	alignas(16) matrix4 view;
	alignas(16) matrix4 proj;
};

const int MAX_FRAMES_IN_FLIGHT = 2;

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

/*
const std::vector<const char*> deviceExtensions = {
VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
*/

const std::vector<const char*> deviceExtensions = {};

const std::vector<const char*> validationLayers = {
"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices {
	uint32_t graphicsFamily;
	uint32_t presentFamily;
	uint32_t graphicsAndComputeFamily;
};

class MyDevice
{

public:

	MyDevice() {
		init();
	}

	void cleanup() {
		vkDestroyCommandPool(device, commandPool, nullptr);
		vkDestroyDevice(device, nullptr);
		//vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
	}

	VkDevice getDevice() {
		return device;
	}

	VkPhysicalDevice getPhysicalDevice() {
		return physicalDevice;
	}
	/*
	VkSurfaceKHR getSurface() {
		return surface;
	}*/

	VkQueue getGraphicsQueue() {
		return graphicsQueue;
	}
	/*
	VkQueue getPresentQueue() {
		return presentQueue;
	}*/

	VkFormat findSupportedFormat(const std::vector<VkFormat>&
		candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice pdevice, VkSurfaceKHR surface);
	QueueFamilyIndices findQueueFamilies();

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, 
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	VkBufferView createBufferView(VkBuffer buffer, VkFormat format, VkDeviceSize range, VkBufferViewCreateFlags flags);

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void createCommandPool();

	VkCommandPool getCommandPool() {
		return commandPool;
	}


	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);


	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void copyImageToBuffer(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

private:
	void init()
	{
		createInstance();
		//createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createCommandPool();
	}

	//void createSurface();
	bool checkValidationLayerSupport();
	void createInstance();
	void createLogicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice physical_device);
	void pickPhysicalDevice();
	bool checkDeviceExtensionSupport(VkPhysicalDevice physical_device);
	//SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physical_device);

	VkInstance instance;
	VkDevice device = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	//VkSurfaceKHR surface;

	VkCommandPool commandPool;

	VkQueue graphicsQueue;
	//VkQueue presentQueue;
};