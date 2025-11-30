#pragma once


#include <irrlicht.h>
#include <vulkan/vulkan.h>

#include <vector>
#include <stdexcept>
#include <unordered_map>

#include "vkDevice.h"

class MyDescriptorSetLayout
{

public:
	MyDescriptorSetLayout(MyDevice* device, const std::vector<VkDescriptorSetLayoutBinding>& bindings) : 
		device{ device }, bindings{ bindings } {

		//std::vector< VkDescriptorSetLayoutBinding > setLayoutBindings{};

		//for (auto pair : bindings) {
		//	setLayoutBindings.push_back(pair.second);
		//}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device->getDevice(), &layoutInfo, nullptr,
			&descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	~MyDescriptorSetLayout() {
		//vkDestroyDescriptorSetLayout(device->getDevice(), descriptorSetLayout, nullptr);
	}

	void cleanup() {
		vkDestroyDescriptorSetLayout(device->getDevice(), descriptorSetLayout, nullptr);
	}

	VkDescriptorSetLayout& getDescriptorSetLayout() { return descriptorSetLayout; }

private:
	MyDevice* device;
	//std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	VkDescriptorSetLayout descriptorSetLayout;

	friend class MyDescriptorWriter;
};

class MyDescriptorPool
{
public:
	MyDescriptorPool(MyDevice* device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes) : device{ device } {

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(maxSets);
		poolInfo.flags = poolFlags;

		if (vkCreateDescriptorPool(device->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	~MyDescriptorPool() {
		//vkDestroyDescriptorPool(device->getDevice(), descriptorPool, nullptr);
	}

	void cleanup() {
		vkDestroyDescriptorPool(device->getDevice(), descriptorPool, nullptr);
	}

	bool allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet) const {

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;

		if (vkAllocateDescriptorSets(device->getDevice(), &allocInfo,
			&descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor set!");
			return false;
		}
		return true;
	}

	void freeDescriptorsSets(std::vector<VkDescriptorSet>& descriptors) {
		vkFreeDescriptorSets(device->getDevice(), descriptorPool, descriptors.size(), descriptors.data());
	}

	void resetPool() {
		vkResetDescriptorPool(device->getDevice(), descriptorPool, 0);
	}

private:
	MyDevice* device;
	VkDescriptorPool descriptorPool;

	friend class MyDescriptorWriter;
};

class MyDescriptorWriter
{
public:
	MyDescriptorWriter(MyDescriptorSetLayout& setLayout, MyDescriptorPool& pool) :
		setLayout{ setLayout }, pool{ pool } {
	}

	void writeBuffer(uint32_t binding, VkDescriptorBufferInfo bufferInfo) {
		VkWriteDescriptorSet descriptorWrite{};

		auto& bindingDescription = setLayout.bindings[binding];

		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//descriptorWrite.dstSet = descriptorSets[i];
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = bindingDescription.descriptorType;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		writes.push_back(descriptorWrite);
	}

	void writeTexelBuffer(uint32_t binding, VkDescriptorBufferInfo bufferInfo, VkBufferView texelBufferView) {
		VkWriteDescriptorSet descriptorWrite{};

		auto& bindingDescription = setLayout.bindings[binding];

		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//descriptorWrite.dstSet = descriptorSets[i];
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = bindingDescription.descriptorType;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pTexelBufferView = &texelBufferView;

		writes.push_back(descriptorWrite);
	}

	void writeImage(uint32_t binding, VkDescriptorImageInfo imageInfo) {
		VkWriteDescriptorSet descriptorWrite{};

		auto& bindingDescription = setLayout.bindings[binding];

		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//descriptorWrite.dstSet = descriptorSets[i];
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = bindingDescription.descriptorType;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		writes.push_back(descriptorWrite);
	}

	void writeImageArray(uint32_t binding, VkDescriptorImageInfo* imageInfo) {
		VkWriteDescriptorSet descriptorWrite{};

		auto& bindingDescription = setLayout.bindings[binding];

		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//descriptorWrite.dstSet = descriptorSets[i];
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = bindingDescription.descriptorType;
		descriptorWrite.descriptorCount = bindingDescription.descriptorCount;
		descriptorWrite.pImageInfo = imageInfo;

		writes.push_back(descriptorWrite);
	}

	bool build(VkDescriptorSet& set) {
		bool success = pool.allocateDescriptorSet(setLayout.getDescriptorSetLayout(), set);

		if (!success)
			return false;

		overwrite(set);

		return true;
	}

	void overwrite(VkDescriptorSet& set) {

		for (auto &write : writes) {
			write.dstSet = set;
		}

		vkUpdateDescriptorSets(pool.device->getDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
	}

private:
	MyDescriptorSetLayout& setLayout;
	MyDescriptorPool& pool;
	std::vector<VkWriteDescriptorSet> writes;
};
