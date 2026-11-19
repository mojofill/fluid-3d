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

    void init();
    void destroy();
    VkCommandBuffer beginSingleTimeCommand();
    void endSingleTimeCommand(VkCommandBuffer commandBuffer);
};