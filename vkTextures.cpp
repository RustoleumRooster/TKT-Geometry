
#include "vkTextures.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <sstream>

bool MyTextures::addTexture(std::string fname) 
{
	if (index_by_name.count(fname) > 0)
		return false;

	VkImage img = createTextureImage(fname);
	VkImageView imgView = createImageView(img);

	textureImageViews.push_back(imgView);

	index_by_name[fname] = textureImageViews.size() - 1;

	return true;
}

VkSampler MyTextures::createTextureSampler()
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

VkImageView MyTextures::createImageView(VkImage img)
{
	return device->createImageView(img, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

VkImage MyTextures::createTextureImage(std::string fname)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(fname.c_str(), &texWidth,
		&texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	//stbi_

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	MyBufferObject stagingBuffer(device, imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer(pixels);

	//std::stringstream ss;
	//ss << "bmpTest" << "bmptest" << ".bmp";

	//generateBitmapImage(pixels, texWidth, texHeight, ss.str().c_str());

	stbi_image_free(pixels);

	//auto textureImageBackInserter = std::back_inserter<std::vector<VkImage>>(textureImages);
	//auto textureImageMemoryBackInserter = std::back_inserter<std::vector<VkDeviceMemory>>(textureImageMemory);

	VkImage img;
	VkDeviceMemory imgMemory;

	device->createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, img,
		imgMemory);

	device->transitionImageLayout(img, VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	device->copyBufferToImage(stagingBuffer.getBuffer(), img,
		static_cast<uint32_t>(texWidth),
		static_cast<uint32_t>(texHeight));

	device->transitionImageLayout(img, VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	textureImages.push_back(img);
	textureImageMemory.push_back(imgMemory);

	std::cout << "loaded image " << fname.c_str() << "\n";

	return img;
}
