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
            VK_FORMAT_R16G16B16A16_SFLOAT,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &densityImages[i].image,
            &densityImages[i].memory
        );
        images->createImageView(
            densityImages[i].image,
            VK_IMAGE_VIEW_TYPE_3D,
            VK_FORMAT_R16G16B16A16_SFLOAT,
            VK_IMAGE_ASPECT_COLOR_BIT,
            &densityImages[i].imageView
        );
    }
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
        &storageImage.image,
        &storageImage.memory
    );
    images->createImageView(storageImage.image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, &storageImage.imageView);
}

void Resources::createXVelocitiesImages(int width, int length, int height) {
    for (int i = 0; i < 2; i++) {
        images->createImage(
            VK_IMAGE_TYPE_3D,
            width + 1,
            length,
            height,
            VK_FORMAT_R32_SFLOAT,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &xVelocitiesImages[i].image,
            &xVelocitiesImages[i].memory
        );
        images->createImageView(
            xVelocitiesImages[i].image,
            VK_IMAGE_VIEW_TYPE_3D,
            VK_FORMAT_R32_SFLOAT,
            VK_IMAGE_ASPECT_COLOR_BIT,
            &xVelocitiesImages[i].imageView
        );
    }
}

void Resources::createYVelocitiesImages(int width, int length, int height) {
    for (int i = 0; i < 2; i++) {
        images->createImage(
            VK_IMAGE_TYPE_3D,
            width,
            length + 1,
            height,
            VK_FORMAT_R32_SFLOAT,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &yVelocitiesImages[i].image,
            &yVelocitiesImages[i].memory
        );
        images->createImageView(
            yVelocitiesImages[i].image,
            VK_IMAGE_VIEW_TYPE_3D,
            VK_FORMAT_R32_SFLOAT,
            VK_IMAGE_ASPECT_COLOR_BIT,
            &yVelocitiesImages[i].imageView
        );
    }
}

void Resources::createZVelocitiesImages(int width, int length, int height) {
    for (int i = 0; i < 2; i++) {
        images->createImage(
            VK_IMAGE_TYPE_3D,
            width,
            length,
            height + 1,
            VK_FORMAT_R32_SFLOAT,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &zVelocitiesImages[i].image,
            &zVelocitiesImages[i].memory
        );
        images->createImageView(
            zVelocitiesImages[i].image,
            VK_IMAGE_VIEW_TYPE_3D,
            VK_FORMAT_R32_SFLOAT,
            VK_IMAGE_ASPECT_COLOR_BIT,
            &zVelocitiesImages[i].imageView
        );
    }
}

void Resources::createSolidMap(int width, int length, int height) {
    images->createImage(
        VK_IMAGE_TYPE_3D,
        width,
        length,
        height,
        VK_FORMAT_R8_UINT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_STORAGE_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &solidMap.image,
        &solidMap.memory
    );
    images->createImageView(
        solidMap.image,
        VK_IMAGE_VIEW_TYPE_3D,
        VK_FORMAT_R8_UINT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &solidMap.imageView
    );
}

void Resources::createPressureMapImages(int width, int length, int height) {
    for (int i = 0; i < 2; i++) {
        images->createImage(
            VK_IMAGE_TYPE_3D,
            width,
            length,
            height,
            VK_FORMAT_R32_SFLOAT,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_STORAGE_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &pressureMapImages[i].image,
            &pressureMapImages[i].memory
        );
        images->createImageView(
            pressureMapImages[i].image,
            VK_IMAGE_VIEW_TYPE_3D,
            VK_FORMAT_R32_SFLOAT,
            VK_IMAGE_ASPECT_COLOR_BIT,
            &pressureMapImages[i].imageView
        );
    }
}

void Resources::createLinearSampler() {
    images->createSampler(&linearSampler, VK_FILTER_LINEAR);
}

VkDescriptorImageInfo Image::imageInfo() {
    return (VkDescriptorImageInfo) {
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL
    };
}

void Resources::destroyImage(Image &image) {
    vkDestroyImage(vko->device.device, image.image, NULL);
    vkDestroyImageView(vko->device.device, image.imageView, NULL);
    vkFreeMemory(vko->device.device, image.memory, NULL);
}

void Resources::destroy() {
    for (int i = 0; i < 2; i++) {
        destroyImage(densityImages[i]);
        destroyImage(xVelocitiesImages[i]);
        destroyImage(yVelocitiesImages[i]);
        destroyImage(zVelocitiesImages[i]);
        destroyImage(pressureMapImages[i]);
    }
    destroyImage(solidMap);
    destroyImage(storageImage);
    vkDestroySampler(vko->device.device, linearSampler, NULL);
}
