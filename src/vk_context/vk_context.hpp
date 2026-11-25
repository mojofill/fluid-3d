#pragma once

#include "device.hpp"
#include "swapchain.hpp"
#include "command_pool.hpp"

class VulkanContext {
public:
    int width;
    int height;
    Device device;
    Swapchain swapchain;
    CommandPool commandPool;

    // information glfw can transport back and forth via window user pointer
    uint32_t framebufferResized = 0;
    bool fullscreen = false;

    uint32_t ping = 0;
    uint32_t frameCount = 0;
    uint32_t fast = 1;

    uint32_t numPressureSolveIteration;
    uint32_t numSubsteps;

    bool paused = false;

    void *engine; // bang!! can just store the raw pointer. isnt that so cool??

    void init();
    void destroy();
    VkCommandBuffer beginSingleTimeCommand();
    void endSingleTimeCommand(VkCommandBuffer commandBuffer);
};