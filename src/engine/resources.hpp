#ifndef RESOURCES_H
#define RESOURCES_H

#include "buffers.hpp"
#include "images.hpp"

class Image {
public:
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory memory;
    
    VkDescriptorImageInfo imageInfo();
};

class Resources {
private:
    VulkanContext *vko;

    Buffers *buffers;
    Images *images;
public:
    Image storageImage;
    Image densityImages[2];
    Image xVelocitiesImages[2];
    Image yVelocitiesImages[2];
    Image zVelocitiesImages[2];
    Image solidMap;
    Image pressureMapImages[2];

    VkSampler linearSampler;

    void init(VulkanContext *vko, Buffers *buffers, Images *images);

    // resource initializers
    void createDensityImages(int width, int length, int height);
    void createStorageImage(int width, int height);
    void createXVelocitiesImages(int width, int length, int height);
    void createYVelocitiesImages(int width, int length, int height);
    void createZVelocitiesImages(int width, int length, int height);
    void createSolidMap(int width, int length, int height);
    void createPressureMapImages(int width, int length, int height);

    void createLinearSampler();

    void destroyImage(Image &image);
    void destroy();
};

#endif