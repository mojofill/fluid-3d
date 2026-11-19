#include "engine.hpp"
#include <iostream>
#include <chrono>

void Engine::init(VulkanContext *vko) {
    this->vko = vko;
    buffers.init(vko);
    images.init(vko);
    resources.init(vko, &buffers, &images);

    glm::vec3 minXYZ = glm::vec3(-10,-10,-10);
    glm::vec3 maxXYZ = glm::vec3( 10, 10, 10);

    minXYZ *= 10;
    maxXYZ *= 10;

    // initialize all resources
    resources.createDensityImages(maxXYZ.x - minXYZ.x, maxXYZ.y - minXYZ.y, maxXYZ.z - minXYZ.z);
    resources.createStorageImage(vko->width, vko->height);

    descriptors.init(vko, &buffers, &images, &resources);
    renderer.init(vko, &buffers, &images, &descriptors, &resources, &recorder);

    Pipelines &pipelines = renderer.getPipelinesRef();

    pipelines.createComputePipeline(&descriptors, &pipelines.computePipeline, &pipelines.computePipelineLayout, "./src/assets/spvs/comp.spv");
    pipelines.createComputePipeline(&descriptors, &pipelines.testComputePipeline, &pipelines.testComputePipelineLayout, "./src/assets/spvs/test.spv");

    camera.init(glm::vec4(0,-200,0,1), glm::vec4(0,1,0,0), glm::vec4(1,0,0,0), glm::vec4(0,0,1,0));
    camera.lastX = vko->width/2;
    camera.lastY = vko->height/2;

    recorder.minXYZ = minXYZ;
    recorder.maxXYZ = maxXYZ;
}

void Engine::destroy() {
    renderer.destroy();
    descriptors.destroy();
    resources.destroy();
    images.destroy();
    buffers.destroy();
}

void Engine::swapPing() {
    ping = (ping + 1) % 2;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    VulkanContext *vko = (VulkanContext*) glfwGetWindowUserPointer(window);
    if (key == GLFW_KEY_F11 && action == GLFW_RELEASE) {
        vko->fullscreen = !vko->fullscreen;
        GLFWmonitor *monitor = vko->fullscreen ? glfwGetPrimaryMonitor() : NULL;
        glfwSetWindowMonitor(window, monitor, 0, 0, vko->width, vko->height, GLFW_DONT_CARE);
    }
}

void Engine::mainLoop() {
    uint32_t frameCount = 0;
    using clock = std::chrono::steady_clock;
    auto lastTime = clock::now();

    glfwSetKeyCallback(vko->device.window, keyCallback);

    while (!glfwWindowShouldClose(vko->device.window)) {
        glfwPollEvents();
        if (glfwGetKey(vko->device.window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(vko->device.window, VK_TRUE);
        }
        // get delta time
        auto currentTime = clock::now();
        std::chrono::duration<float, std::milli> deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        float dtMs = deltaTime.count(); // e.g., 16.6667f
        float dt = dtMs / 1000.0f;

        dt = fmin(dt, 0.1);

        camera.processEvents(vko->device.window, dt);
        // upload the uniform buffer
        UniformBufferObject uni {
            .camera = camera.toCameraUBO(),
            .minXYZ = glm::vec4(recorder.minXYZ, 0),
            .maxXYZ = glm::vec4(recorder.maxXYZ, 0)
        };
        memcpy(buffers.uniformBuffersMapped[ping], &uni, sizeof(uni));
        frameCount++;
        renderer.draw(&ping);
    }
}
