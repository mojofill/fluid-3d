#ifndef RESOURCES_H
#define RESOURCES_H

#include "buffers.hpp"
#include "images.hpp"

class Resources {
private:
    VulkanContext *vko;

    Buffers *buffers;
    Images *images;
public:
    VkImage storageImage;
	VkDeviceMemory storageImageMemory;
	VkImageView storageImageView;
	VkSampler storageSampler;

    VkImage densityImages[2];
    VkImageView densityImageViews[2];
    VkDeviceMemory densityMemories[2];
    VkSampler densitySampler; // necessary for trilinear interpolation

    void init(VulkanContext *vko, Buffers *buffers, Images *images);

    // resource initializers
    void createDensityImages(int width, int length, int height);
    void createStorageImage(int width, int height);

    void destroy();
};

#endif