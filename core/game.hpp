//
// Created by sabrina on 10/2/19.
//

#ifndef VULKAN_ENGINE_GAME_HPP
#define VULKAN_ENGINE_GAME_HPP

#include <string>
#include <array>
#include <vulkan/vulkan.hpp>

namespace core {
    struct Game {
        std::string title;
        uint32_t majorVersion = 1;
        uint32_t minorVersion = 0;
        uint32_t patchVersion = 0;

        std::array<uint32_t, 3> makeVersion() {
            return VK_MAKE_VERSION(majorVersion, minorVersion, patchVersion);
        }

        vk::ApplicationInfo makeAppInfo() {
            return vk::ApplicationInfo {
                    title,
                    makeVersion(),
                    "Untitled Engine",
                    VK_MAKE_VERSION(0, 1, 0),
                    VK_MAKE_VERSION(1, 0, VK_HEADER_VERSION)
            }
        }
    };
}

#endif //VULKAN_ENGINE_GAME_HPP
