#include "engine.hpp"
#include <iostream>
#include <chrono>

#define GRAVITY 0.0f
#define DELTA_TIME 0.01
#define WEIGHT_SOR 1.75f
#define NUM_PRESSURE_SOLVE_ITERATIONS 10
#define FLUID_DENSITY 1000.0f
#define NUM_SUBSTEPS 2

void Engine::init(VulkanContext *vko) {
    this->vko = vko;
    vko->engine = this;
    vko->numSubsteps = NUM_SUBSTEPS;
    vko->numPressureSolveIteration = NUM_PRESSURE_SOLVE_ITERATIONS;
    buffers.init(vko);
    images.init(vko);
    resources.init(vko, &buffers, &images);

    glm::vec3 minXYZ = glm::vec3(-10,-10,-10);
    glm::vec3 maxXYZ = glm::vec3( 10, 10, 10);

    float s = 25;

    minXYZ *= s;
    maxXYZ *= s;

    glm::ivec3 dimensions = maxXYZ - minXYZ;

    // initialize all resources
    resources.createDensityImages(maxXYZ.x - minXYZ.x, maxXYZ.y - minXYZ.y, maxXYZ.z - minXYZ.z);
    resources.createStorageImage(vko->width, vko->height);
    resources.createXVelocitiesImages(dimensions.x, dimensions.y, dimensions.z);
    resources.createYVelocitiesImages(dimensions.x, dimensions.y, dimensions.z);
    resources.createZVelocitiesImages(dimensions.x, dimensions.y, dimensions.z);
    resources.createLinearSampler();
    resources.createSolidMap(dimensions.x, dimensions.y, dimensions.z);
    resources.createPressureMapImages(dimensions.x, dimensions.y, dimensions.z);

    descriptors.init(vko, &buffers, &images, &resources);
    renderer.init(vko, &buffers, &images, &descriptors, &resources, &recorder);

    Pipelines &pipelines = renderer.getPipelinesRef();

    pipelines.createComputePipeline(
        &descriptors,
        &pipelines.raymarchDensityPipeline.pipeline,
        &pipelines.raymarchDensityPipeline.layout,
        "./src/assets/spvs/raymarchDensity.spv"
    );
    pipelines.createComputePipeline(
        &descriptors,
        &pipelines.initializationComputePipeline.pipeline,
        &pipelines.initializationComputePipeline.layout,
        "./src/assets/spvs/initialization.spv"
    );
    pipelines.createComputePipeline(
        &descriptors,
        &pipelines.applyExternalForcesPipeline.pipeline,
        &pipelines.applyExternalForcesPipeline.layout,
        "./src/assets/spvs/applyExternalForces.spv"
    );
    pipelines.createComputePipeline(
        &descriptors,
        &pipelines.advectVelocitiesPipeline.pipeline,
        &pipelines.advectVelocitiesPipeline.layout,
        "./src/assets/spvs/advectVelocities.spv"
    );
    pipelines.createComputePipeline(
        &descriptors,
        &pipelines.solvePressurePipeline.pipeline,
        &pipelines.solvePressurePipeline.layout,
        "./src/assets/spvs/solvePressure.spv"
    );
    pipelines.createComputePipeline(
        &descriptors,
        &pipelines.applyPressureForcePipeline.pipeline,
        &pipelines.applyPressureForcePipeline.layout,
        "./src/assets/spvs/applyPressureForce.spv"
    );
    pipelines.createComputePipeline(
        &descriptors,
        &pipelines.advectSmokePipeline.pipeline,
        &pipelines.advectSmokePipeline.layout,
        "./src/assets/spvs/advectSmoke.spv"
    );
    pipelines.createComputePipeline(
        &descriptors,
        &pipelines.persistDataPipeline.pipeline,
        &pipelines.persistDataPipeline.layout,
        "./src/assets/spvs/persistData.spv"
    );

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

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    VulkanContext *vko = (VulkanContext*) glfwGetWindowUserPointer(window);
    if (key == GLFW_KEY_F11 && action == GLFW_RELEASE) {
        vko->fullscreen = !vko->fullscreen;
        GLFWmonitor *monitor = vko->fullscreen ? glfwGetPrimaryMonitor() : NULL;
        glfwSetWindowMonitor(window, monitor, 0, 0, vko->width, vko->height, GLFW_DONT_CARE);
    }
    if (key == GLFW_KEY_P && action == GLFW_RELEASE) {
        vko->paused = !vko->paused;
    }
    if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
        vko->paused = true;
        ((Engine*) vko->engine)->initializeData(); // this is disgusting LOL
    }
    if (key == GLFW_KEY_L && action == GLFW_RELEASE) {
        vko->ping = 1 - vko->ping;
    }
    if (key == GLFW_KEY_F && action == GLFW_RELEASE) {
        vko->fast = 1 - vko->fast;
    }
}

void Engine::initializeData() {
    VkCommandBuffer commandBuffer = vko->beginSingleTimeCommand();
    for (int i = 0; i < 2; i++) {
        recorder.initializationComputeShader(vko->ping, commandBuffer);
        vko->ping = 1 - vko->ping;
        recorder.persistDataComputeShader(vko->ping, commandBuffer);
        vko->ping = 1 - vko->ping;
    }
    vko->endSingleTimeCommand(commandBuffer);
}

void Engine::mainLoop() {
    using clock = std::chrono::steady_clock;
    auto lastTime = clock::now();

    glfwSetKeyCallback(vko->device.window, keyCallback);

    uint32_t currentFrame = 0;

    while (!glfwWindowShouldClose(vko->device.window)) {
        glfwPollEvents();
        if (glfwGetKey(vko->device.window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(vko->device.window, VK_TRUE);
        }
        // get delta time
        auto currentTime = clock::now();
        std::chrono::duration<float, std::milli> deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        float dtMs = deltaTime.count();
        float dt = dtMs / 1000.0f;

        // bro lowkey if it work it works, dont worry about it
        if (vko->frameCount == 1) {
            initializeData();
        }

        dt = fmin(dt, 0.1);

        camera.processEvents(vko->device.window, dt);
        // upload the uniform buffer
        UniformBufferObject uni {
            .camera = camera.toCameraUBO(),
            .minXYZ = glm::vec4(recorder.minXYZ, 0),
            .maxXYZ = glm::vec4(recorder.maxXYZ, 0),
            .frameCount = vko->frameCount,
            .g = GRAVITY,
            .dt = DELTA_TIME / (float) NUM_SUBSTEPS,
            .weightSOR = WEIGHT_SOR,
            .density = FLUID_DENSITY,
            .fast = vko->fast
        };
        memcpy(buffers.uniformBuffersMapped[currentFrame], &uni, sizeof(uni));
        vko->frameCount++;
        renderer.draw(currentFrame);
    }
}
