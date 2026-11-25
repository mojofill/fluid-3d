#pragma once

#include "../vk_context/vk_context.hpp"
#include "buffers.hpp"
#include "images.hpp"
#include "descriptors.hpp"
#include "resources.hpp"
#include "../renderer/renderpass.hpp"
#include "../renderer/pipelines.hpp"
#include "../renderer/framebuffers.hpp"

class Recorder {
private:
    VulkanContext *vko;
    Images *images;
    Descriptors *descriptors;
    Resources *resources;
    RenderPass *renderPass;
    Pipelines *pipelines;
    Framebuffers *framebuffers;
    Buffers *buffers;

    VkViewport viewport;
    VkRect2D scissor;

    void createCommandBuffers();
public:
    glm::vec3 minXYZ;
    glm::vec3 maxXYZ;

    VkCommandBuffer *commandBuffers;
    void init(
        VulkanContext *vko,
        Images *images,
        Descriptors *descriptors,
        Resources *resources,
        RenderPass *renderPass,
        Pipelines *pipelines,
        Framebuffers *Framebuffers,
        Buffers *buffers
    );
    void destroy();

    // compute shader functions
    void runComputeShader(
        uint32_t ping,
        VkCommandBuffer &commandBuffer,
        Pipeline &computePipeline,
        uint32_t groupCountX,
        uint32_t groupCountY,
        uint32_t groupCountZ
    );
    void addImageMemoryBarriers(uint32_t ping, VkCommandBuffer &commandBuffer);
    
    void raymarchDensityFieldComputeShader(uint32_t ping, VkCommandBuffer &commandBuffer);
    void initializationComputeShader(uint32_t ping, VkCommandBuffer &commandBuffer);
    void applyExternalForcesComputeShader(uint32_t ping, VkCommandBuffer &commandBuffer);
    void advectVelocitiesComputeShader(uint32_t ping, VkCommandBuffer &commandBuffer);
    void solvePressureComputeShader(uint32_t ping, VkCommandBuffer &commandBuffer);
    void applyPressureForceComputeShader(uint32_t ping, VkCommandBuffer &commandBuffer);
    void advectSmokeComputeShader(uint32_t ping, VkCommandBuffer &commandBuffer);
    void persistDataComputeShader(uint32_t ping, VkCommandBuffer &commandBuffer);
    
    void drawStorageImage(uint32_t ping, uint32_t i);
    void recordCommands();
    void resetCommands();
};
