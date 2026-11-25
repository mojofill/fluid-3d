#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include "../vk_context/vk_context.hpp"
#include "buffers.hpp"
#include "images.hpp"
#include "resources.hpp"
#include "camera.hpp"

typedef struct UniformBufferObject {
    CameraUBO camera;
    glm::vec4 minXYZ;
    glm::vec4 maxXYZ;
    uint32_t frameCount;
    float g;
    float dt;
    float weightSOR;
    float density;
    uint32_t fast;
} UniformBufferObject;

typedef struct PushConstants {
    uint32_t ping;
} PushConstants;

class Descriptors {
private:
    VulkanContext *vko;
    Buffers *buffers;
    Images *images;
    Resources *resources;
    void addDescriptorSetLayoutBinding(
        std::vector<VkDescriptorSetLayoutBinding> &bindings,
        std::vector<VkDescriptorPoolSize> &poolSizes,
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        const VkSampler *pImmutableSampler
    );
    void addWriteBufferDescriptorSet(
        std::vector<VkWriteDescriptorSet> &desciptorWrites,
        VkDescriptorSet descriptorSet,
        uint32_t dstBinding,
        VkDescriptorType descriptorType,
        VkDescriptorBufferInfo *pBufferInfo
    );
    void addWriteImageDescriptorSet(
        std::vector<VkWriteDescriptorSet> &descriptorWrites,
        VkDescriptorSet descriptorSet,
        uint32_t dstBinding,
        VkDescriptorType descriptorType,
        VkDescriptorImageInfo *pImageInfo
    );
    void createDescriptorSetResources();
    void createUniformBuffer();
    void createDescriptorSets();
public:
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSets[2];
    void init(VulkanContext *vko, Buffers *buffers, Images *images, Resources *resources);
    void destroy();
};
#endif