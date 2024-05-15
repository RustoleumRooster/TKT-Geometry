#pragma once


#include <vulkan/vulkan.h>

#include <vector>
#include <stdexcept>
#include "vkDevice.h"
#include <iostream>

class MyBufferObject {
public:
	MyBufferObject(MyDevice* device_, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment) : device(device_) {

		//std::cout << instanceSize << "\n";
		//std::cout << getAlignment(instanceSize, minOffsetAlignment) << "\n";
		bufferSize = instanceCount * getAlignment(instanceSize,minOffsetAlignment);

		device->createBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, bufferMemory);

	}

	~MyBufferObject() {
		vkDestroyBuffer(device->getDevice(), buffer, nullptr);
		vkFreeMemory(device->getDevice(), bufferMemory, nullptr);

	}

	void writeToBuffer(void* data, VkDeviceSize = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	void readFromBuffer(void* data, VkDeviceSize = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

	VkBuffer getBuffer() { return buffer; }
	void* getMappedMemory() { return mapped_data; }

	void map();
	void unmap();

private:
	static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
	{
		if( instanceSize > minOffsetAlignment)
			return minOffsetAlignment * ((instanceSize / minOffsetAlignment) + (instanceSize % minOffsetAlignment == 0 ? 0 : 1));
		else 
			return minOffsetAlignment;
	}

	MyDevice* device;
	VkBuffer buffer;
	void* mapped_data = nullptr;
	VkDeviceMemory bufferMemory;
	VkDeviceSize bufferSize;

};