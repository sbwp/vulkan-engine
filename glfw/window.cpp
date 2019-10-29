//
// Created by sabrina on 10/6/19.
//

#include "window.hpp"
#include "../logger/logger.hpp"

namespace glfw{
	void Window::framebufferResizeCallback(GLFWwindow* c_window, int width, int height) {
		auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(c_window));
		window->resized = true;
	}

    Window::Window(int width, int height, std::string const& title) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    Window::~Window() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    bool Window::shouldClose() {
        return glfwWindowShouldClose(window);
    }

    vk::SurfaceKHR Window::createSurface(vk::Instance& instance) {
        VkSurfaceKHR c_surface;
        Logger::assertTrue(glfwCreateWindowSurface(VkInstance(instance), window, nullptr, &c_surface) == VK_SUCCESS,
                           "Window surface creation failed.");
        return vk::SurfaceKHR(c_surface);
    }

    vk::Extent2D Window::getFramebufferSize() {
        int width, height;
		glfwGetFramebufferSize(window, &width, &height); // Just to avoid the extra glfwWaitEvents when nonzero

        while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
        }
        return {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
    }

	bool Window::shouldResize() {
		return resized;
	}

	void Window::handleResize() {
		resized = false;
	}

	void appendRequiredExtensions(std::vector<const char *> &instanceExtensions) {
        uint32_t glfwRequiredExtensionsCount;
        auto glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&glfwRequiredExtensionsCount);
        for (uint32_t i = 0; i < glfwRequiredExtensionsCount; ++i) {
            instanceExtensions.push_back(glfwRequiredExtensions[i]);
        }
    }

    void tick() {
        glfwPollEvents();
    }
}
