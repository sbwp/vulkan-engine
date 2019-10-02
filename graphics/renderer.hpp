//
// Created by sabrina on 10/1/19.
//

#ifndef VULKAN_ENGINE_RENDERER_HPP
#define VULKAN_ENGINE_RENDERER_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <vulkan/vulkan.h>
#include "../core/game.hpp"

namespace Graphics {
    class Renderer {
    public:
        Renderer(core::Game game);

    private:
        std::vector<const char*> instanceExtensions {
            VK_KHR_SURFACE_EXTENSION_NAME
        };
        const std::vector<const char*> deviceExtensions {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        std::vector<const char*> validationLayers {};

    };
}

#endif //VULKAN_ENGINE_RENDERER_HPP
