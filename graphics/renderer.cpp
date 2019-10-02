//
// Created by sabrina on 10/1/19.
//

#include "renderer.hpp"
#include "../logger/logger.hpp"
#include "validation.hpp"

namespace Graphics {
    Renderer::Renderer(core::Game game) {
        setupValidationLayers(instanceExtensions, validationLayers);

        auto appInfo = game.makeAppInfo();
        vk::InstanceCreateInfo createInfo {
            vk::InstanceCreateFlags(),
            &appInfo,
            static_cast<uint32_t>(validationLayers.size()),
            validationLayers.data(),
            static_cast<uint32_t>(instanceExtensions.size()),
            instanceExtensions.data()
        };

        setupValidationCallback();
    }
}
