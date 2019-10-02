//
// Created by sabrina on 10/2/19.
//

#ifndef VULKAN_ENGINE_VALIDATION_HPP
#define VULKAN_ENGINE_VALIDATION_HPP

#include <string>
#include <vector>

namespace Graphics {
    void setupValidationLayers(std::vector<const char*>& instanceExtensions, std::vector<const char*>& validationLayers);

    void setupValidationCallback();
}


#endif //VULKAN_ENGINE_VALIDATION_HPP
