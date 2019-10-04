#ifdef DEBUG
#define ENABLE_VK_VALIDATION
#endif

#include <iostream>
#include "graphics/renderer.hpp"

int main() {
    Core::Game game{ "Michael's Toys", 0, 1, 0};
    Graphics::Renderer renderer(game);
    renderer.start();
    return 0;
}