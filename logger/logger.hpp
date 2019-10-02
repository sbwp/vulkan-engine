//
// Created by sabrina on 10/1/19.
//

#ifndef VULKAN_ENGINE_LOGGER_HPP
#define VULKAN_ENGINE_LOGGER_HPP

#include <stdexcept>
#include <iostream>

namespace Logger {
    class error : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    template<typename T>
    void log (T t) {
        std::cout << t << std::endl;
    }

    template<typename T, typename... Args>
    void log (T t, Args... args) {
        std::cout << t;
        log(args...);
    }
}

#endif //VULKAN_ENGINE_LOGGER_HPP
