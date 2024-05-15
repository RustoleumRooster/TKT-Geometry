#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <unordered_map>
#include <stdexcept>

#include "vkBufferObject.h"
#include "vkDevice.h"

class MyTextures
{
public:
	MyTextures(MyDevice* device) : device{ device } {
		defaultSampler = createTextureSampler();
	}

	bool addTexture(std::string fname);

	void cleanup() {

		vkDestroySampler(device->getDevice(), defaultSampler, nullptr);

		for (auto& imgView : textureImageViews)
			vkDestroyImageView(device->getDevice(), imgView, nullptr);

		for(auto& img : textureImages)
			vkDestroyImage(device->getDevice(), img, nullptr);

		for (auto& imgMem : textureImageMemory)
			vkFreeMemory(device->getDevice(), imgMem, nullptr);
	}

	VkSampler getDefaultSampler() {
		return defaultSampler;
	}

	VkImageView getImageView(std::string fname)
	{
		if (index_by_name.count(fname) > 0) {
			int idx = index_by_name[fname];
			return textureImageViews[idx];
		}
		else {
			throw std::runtime_error("ERR: requested texture could not be found!");
		}

	}

private:

	VkImage createTextureImage(std::string fname);
	VkSampler createTextureSampler();
	VkImageView createImageView(VkImage img);

	MyDevice* device = NULL;

	std::vector<VkImage> textureImages;
	std::vector<VkDeviceMemory> textureImageMemory;
	std::vector<VkImageView> textureImageViews;

	VkSampler defaultSampler;

	std::unordered_map<std::string, int> index_by_name;
};