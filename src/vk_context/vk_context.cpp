#include "vk_context.hpp"
#include "../utils.hpp"

void VulkanContext::init() {
    Profiler profiler;
    device.init(width, height);
    swapchain.init(&device);
    profiler.mark("swapchain initialization");
    commandPool.init(&device);
    profiler.mark("command pool initialization");
}

void VulkanContext::destroy() {
    commandPool.destroy();
    swapchain.destroy();
    device.destroy();
}

VkCommandBuffer VulkanContext::beginSingleTimeCommand() {
    return commandPool.beginSingleTimeCommand(&device, &commandPool);
}

void VulkanContext::endSingleTimeCommand(VkCommandBuffer commandBuffer) {
    commandPool.endSingleTimeCommand(&device, &commandPool, commandBuffer);
}
