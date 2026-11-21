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
public:
    void init(VulkanContext *vko);
    void initializeData();
    void mainLoop();
    void destroy();
};

#endif