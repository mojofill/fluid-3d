#include "resources.hpp"
#include <iostream>

void Resources::init(VulkanContext *vko, Buffers *buffers, Images *images) {
    this->vko = vko;
    this->buffers = buffers;
    this->images = images;
}

// width = x, length = y, height = z
void Resources::createDensityImages(int width, int length, int height) {
    for (int i = 0; i < 2; i++) {
        images->createImage(
            VK_IMAGE_TYPE_3D,
            width,
            length,
            height,
            VK_FORMAT_R32_SFLOAT,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &densityImages[i],
            &densityMemories[i]
        );
        images->createImageView(densityImages[i], VK_IMAGE_VIEW_TYPE_3D, VK_FORMAT_R32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, &densityImageViews[i]);
    }
    images->createSampler(&densitySampler, VK_FILTER_LINEAR);
}

void Resources::createStorageImage(int width, int height) {
    images->createImage( // create storage image for a fullscreen quad
        VK_IMAGE_TYPE_2D,
        width,
        height,
        1,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &storageImage,
        &storageImageMemory
    );
    images->createImageView(storageImage, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, &storageImageView);
    images->createSampler(&storageSampler, VK_FILTER_LINEAR);
}

void Resources::destroy() {
    for (int i = 0; i < 2; i++) {
        vkDestroyImageView(vko->device.device, densityImageViews[i], NULL);
        vkDestroyImage(vko->device.device, densityImages[i], NULL);
        vkFreeMemory(vko->device.device, densityMemories[i], NULL);
    }
    vkDestroySampler(vko->device.device, densitySampler, NULL);
    vkDestroyImage(vko->device.device, storageImage, NULL);
    vkFreeMemory(vko->device.device, storageImageMemory, NULL);
    vkDestroyImageView(vko->device.device, storageImageView, NULL);
    vkDestroySampler(vko->device.device, storageSampler, NULL);
}
