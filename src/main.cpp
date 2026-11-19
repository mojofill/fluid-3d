#include "engine/engine.hpp"

int main() {
    VulkanContext vko;
    vko.init();
    Engine engine;
    engine.init(&vko);
    engine.mainLoop();
    engine.destroy();
    vko.destroy();
    return 0;
}
