//
// Created by sabrina on 10/6/19.
//

#ifndef VULKAN_ENGINE_WINDOW_HPP
#define VULKAN_ENGINE_WINDOW_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace glfw{
    class Window {
    public:
        Window(int width, int height, std::string const& title);
        ~Window();
        bool shouldClose();
        vk::SurfaceKHR createSurface(vk::Instance& instance);
        vk::Extent2D getFramebufferSize();
    private:
        GLFWwindow* window;
    };

    void appendRequiredExtensions(std::vector<const char*>& extensions);
    void tick();
}

#endif //VULKAN_ENGINE_WINDOW_HPP
