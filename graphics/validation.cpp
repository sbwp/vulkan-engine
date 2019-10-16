//
// Created by sabrina on 10/2/19.
//

#include <vulkan/vulkan.hpp>
#include "validation.hpp"

namespace Graphics {
	void
	setupValidationLayers(std::vector<const char*>& instanceExtensions, std::vector<const char*>& validationLayers) {
#ifdef DEBUG
		std::vector<char const*> debugInstanceExtensions {
				VK_EXT_DEBUG_REPORT_EXTENSION_NAME
		};

		instanceExtensions.insert(end(instanceExtensions), begin(debugInstanceExtensions), end(debugInstanceExtensions));

		validationLayers = std::vector<char const*> {
				"VK_LAYER_LUNARG_standard_validation"
		};
#endif // DEBUG
	}

	void setupValidationCallback() {
#ifdef DEBUG

#endif // DEBUG
	}
}
