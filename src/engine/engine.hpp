#ifndef ENGINE_H
#define ENGINE_H

#include "../vk_context/vk_context.hpp"
#include "descriptors.hpp"
#include "buffers.hpp"
#include "images.hpp"
#include "resources.hpp"
#include "../renderer/renderer.hpp"
#include "camera.hpp"

class Engine {
private:
    VulkanContext *vko;
    Buffers buffers;
    Images images;
    Resources resources;
    Descriptors descriptors;
    Recorder recorder;
    Renderer renderer;
    
    Camera camera;

    uint32_t ping = 0;
public:
    void init(VulkanContext *vko);
    void mainLoop();
    void destroy();
    void swapPing();
};

#endif