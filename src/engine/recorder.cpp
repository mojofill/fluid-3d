#include "recorder.hpp"
#include <iostream>

void Recorder::init(
    VulkanContext *vko,
    Images *images,
    Descriptors *descriptors,
    Resources *resources,
    RenderPass *renderPass,
    Pipelines *pipelines,
    Framebuffers *framebuffers,
    Buffers *buffers
) {
    this->vko = vko;
    this->images = images;
    this->descriptors = descriptors;
    this->resources = resources;
    this->renderPass = renderPass;
    this->pipelines = pipelines;
    this->framebuffers = framebuffers;
    this->buffers = buffers;
    createCommandBuffers();
}

void Recorder::createCommandBuffers() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vko->commandPool.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    commandBuffers = (VkCommandBuffer*) malloc(sizeof(VkCommandBuffer) * vko->swapchain.swapchainImageCount);

    for (uint32_t i = 0; i < vko->swapchain.swapchainImageCount; i++) {
        vkAllocateCommandBuffers(vko->device.device, &allocInfo, &commandBuffers[i]);
    }
}

void Recorder::resetCommands() {
    for (uint32_t i = 0; i < vko->swapchain.swapchainImageCount; i++) {
        vkResetCommandBuffer(commandBuffers[i], 0);
    }
}

void Recorder::drawStorageImage(uint32_t ping, uint32_t i) {
    VkRenderPassBeginInfo renderPassInfoCmd{}; // most basic command is starting a render pass
    renderPassInfoCmd.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfoCmd.renderPass = renderPass->renderPass;
    renderPassInfoCmd.framebuffer = framebuffers->framebuffers[i];
    renderPassInfoCmd.renderArea.offset = (VkOffset2D){0,0};
    renderPassInfoCmd.renderArea.extent = vko->device.surfaceCapabilities.currentExtent;

    VkClearValue clearColors[2] = {{0}, {0}};
    clearColors[0].color = (VkClearColorValue) {0.0f, 0.0f, 0.0f, 0.0f};
    clearColors[1].depthStencil = (VkClearDepthStencilValue) {1.0f, 0};

    renderPassInfoCmd.clearValueCount = 2;
    renderPassInfoCmd.pClearValues = clearColors;

    vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfoCmd, VK_SUBPASS_CONTENTS_INLINE);

    // here is graphipcs pipeline work
    vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines->graphicsPipeline.pipeline);
    vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines->graphicsPipeline.layout, 0, 1, &descriptors->descriptorSets[ping], 0, NULL);

    // specified viewport + scissor to be dynamic => must explicity set them here
    viewport = (VkViewport) {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) vko->device.surfaceCapabilities.currentExtent.width;
    viewport.height = (float) vko->device.surfaceCapabilities.currentExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

    // must specify scissor as well b/c we've set pipeline to be in dynamic state
    scissor = (VkRect2D) {0};
    scissor.offset = (VkOffset2D){0,0};
    scissor.extent = vko->device.surfaceCapabilities.currentExtent;
    vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

    vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &buffers->vertexBuffer, (VkDeviceSize[]) {0});
    vkCmdDraw(commandBuffers[i], buffers->vertexCount, 1, 0, 0);
}

void Recorder::runComputeShader(
    uint32_t ping,
    VkCommandBuffer &commandBuffer,
    Pipeline &computePipeline,
    uint32_t groupCountX,
    uint32_t groupCountY,
    uint32_t groupCountZ
) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline.pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline.layout, 0, 1, &descriptors->descriptorSets[ping], 0, NULL);
    vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void Recorder::addImageMemoryBarriers(uint32_t ping, VkCommandBuffer &commandBuffer) {
    Compute::createImageMemoryBarrier(resources->densityImages[1 - ping].image, commandBuffer);
    Compute::createImageMemoryBarrier(resources->xVelocitiesImages[1 - ping].image, commandBuffer);
    Compute::createImageMemoryBarrier(resources->yVelocitiesImages[1 - ping].image, commandBuffer);
    Compute::createImageMemoryBarrier(resources->zVelocitiesImages[1 - ping].image, commandBuffer);
    Compute::createImageMemoryBarrier(resources->pressureMapImages[1 - ping].image, commandBuffer);
}

void Recorder::raymarchDensityFieldComputeShader(uint32_t ping, VkCommandBuffer &commandBuffer) {
    runComputeShader(ping, commandBuffer, pipelines->raymarchDensityPipeline, (vko->width + 15) / 16, (vko->height + 15) / 16, 1);
    Compute::createImageMemoryBarrier(resources->storageImage.image, commandBuffer);
}

void Recorder::initializationComputeShader(uint32_t ping, VkCommandBuffer &commandBuffer) {
    runComputeShader(
        ping,
        commandBuffer,
        pipelines->initializationComputePipeline,
        (maxXYZ.x - minXYZ.x + 7) / 8,
        (maxXYZ.y - minXYZ.y + 7) / 8,
        (maxXYZ.z - minXYZ.z + 7) / 8
    );
    addImageMemoryBarriers(ping, commandBuffer);
}

void Recorder::applyExternalForcesComputeShader(uint32_t ping, VkCommandBuffer &commandBuffer) {
    runComputeShader(
        ping,
        commandBuffer,
        pipelines->applyExternalForcesPipeline,
        (maxXYZ.x - minXYZ.x + 7) / 8,
        (maxXYZ.y - minXYZ.y + 7) / 8,
        (maxXYZ.z - minXYZ.z + 7) / 8
    );
    addImageMemoryBarriers(ping, commandBuffer);
}

void Recorder::advectVelocitiesComputeShader(uint32_t ping, VkCommandBuffer &commandBuffer) {
    runComputeShader(
        ping,
        commandBuffer,
        pipelines->advectVelocitiesPipeline,
        (maxXYZ.x - minXYZ.x + 7) / 8,
        (maxXYZ.y - minXYZ.y + 7) / 8,
        (maxXYZ.z - minXYZ.z + 7) / 8
    );
    addImageMemoryBarriers(ping, commandBuffer);
}

void Recorder::solvePressureComputeShader(uint32_t ping, VkCommandBuffer &commandBuffer) {
    // need push constants
    PushConstants constants = {
        .ping = ping
    };
    vkCmdPushConstants(
        commandBuffer,
        pipelines->solvePressurePipeline.layout,
        VK_SHADER_STAGE_COMPUTE_BIT,
        0,
        sizeof(PushConstants),
        &constants
    );
    runComputeShader(
        ping,
        commandBuffer,
        pipelines->solvePressurePipeline,
        (maxXYZ.x - minXYZ.x + 7) / 8,
        (maxXYZ.y - minXYZ.y + 7) / 8,
        (maxXYZ.z - minXYZ.z + 7) / 8
    );
    addImageMemoryBarriers(ping, commandBuffer);
}

void Recorder::applyPressureForceComputeShader(uint32_t ping, VkCommandBuffer &commandBuffer) {
    runComputeShader(
        ping,
        commandBuffer,
        pipelines->applyPressureForcePipeline,
        (maxXYZ.x - minXYZ.x + 7) / 8,
        (maxXYZ.y - minXYZ.y + 7) / 8,
        (maxXYZ.z - minXYZ.z + 7) / 8
    );
    addImageMemoryBarriers(ping, commandBuffer);
}

void Recorder::advectSmokeComputeShader(uint32_t ping, VkCommandBuffer &commandBuffer) {
    runComputeShader(
        ping,
        commandBuffer,
        pipelines->advectSmokePipeline,
        (maxXYZ.x - minXYZ.x + 7) / 8,
        (maxXYZ.y - minXYZ.y + 7) / 8,
        (maxXYZ.z - minXYZ.z + 7) / 8
    );
    addImageMemoryBarriers(ping, commandBuffer);
}

void Recorder::persistDataComputeShader(uint32_t ping, VkCommandBuffer &commandBuffer) {
    runComputeShader(
        ping,
        commandBuffer,
        pipelines->persistDataPipeline,
        (maxXYZ.x - minXYZ.x + 7) / 8,
        (maxXYZ.y - minXYZ.y + 7) / 8,
        (maxXYZ.z - minXYZ.z + 7) / 8
    );
    addImageMemoryBarriers(ping, commandBuffer);
}

void Recorder::recordCommands() {
    // record commands (per swapchain image)
    uint32_t old_ping = vko->ping;
    for (uint32_t i = 0; i < vko->swapchain.swapchainImageCount; i++) {
        vko->ping = old_ping;
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

        if (!vko->paused) {
            for (int step = 0; step < vko->numSubsteps; step++) {
                // upload velocities
                persistDataComputeShader(vko->ping, commandBuffers[i]);
                vko->ping = 1 - vko->ping;

                // apply external forces
                applyExternalForcesComputeShader(vko->ping, commandBuffers[i]);
                vko->ping = 1 - vko->ping; // swap ping

                // advect velocities
                advectVelocitiesComputeShader(vko->ping, commandBuffers[i]);
                vko->ping = 1 - vko->ping; // swap ping

                // solve pressure
                for (int iter = 0; iter < vko->numPressureSolveIteration; iter++) {
                    solvePressureComputeShader(iter % 2, commandBuffers[i]);
                    vko->ping = 1 - vko->ping; // swap ping
                }

                // apply pressure force
                applyPressureForceComputeShader(vko->ping, commandBuffers[i]);
                vko->ping = 1 - vko->ping; // swap ping

                // advect density
                advectSmokeComputeShader(vko->ping, commandBuffers[i]);
                vko->ping = 1 - vko->ping;
            }
        }

        // ray march density
        raymarchDensityFieldComputeShader(vko->ping, commandBuffers[i]);

        // draw the full screen quad
        drawStorageImage(vko->ping, i);

        vkCmdEndRenderPass(commandBuffers[i]);
        vkEndCommandBuffer(commandBuffers[i]);
    }
}

void Recorder::destroy() {
    // nothing to destroy
}
