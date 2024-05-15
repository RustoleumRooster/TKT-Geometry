#include "vkBufferObject.h"

void MyBufferObject::writeToBuffer(void* data, VkDeviceSize unused, VkDeviceSize offset)
{
	vkMapMemory(device->getDevice(), bufferMemory, 0, bufferSize, 0, &mapped_data);

	memcpy(mapped_data, data, bufferSize);

	vkUnmapMemory(device->getDevice(), bufferMemory);
}

void MyBufferObject::readFromBuffer(void* data, VkDeviceSize unused, VkDeviceSize offset)
{
	vkMapMemory(device->getDevice(), bufferMemory, 0, bufferSize, 0, &mapped_data);

	memcpy(data, mapped_data, bufferSize);

	vkUnmapMemory(device->getDevice(), bufferMemory);
}

void MyBufferObject::map()
{
	vkMapMemory(device->getDevice(), bufferMemory, 0, bufferSize, 0, &mapped_data);
}

void MyBufferObject::unmap()
{
	vkUnmapMemory(device->getDevice(), bufferMemory);
}
