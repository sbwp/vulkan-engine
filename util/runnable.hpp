//
// Created by sabrina on 10/4/19.
//

#ifndef VULKAN_ENGINE_RUNNABLE_HPP
#define VULKAN_ENGINE_RUNNABLE_HPP

#include <functional>

namespace Util {
    class Runnable {
    public:
        virtual ~Runnable() = default;
        void start();
    private:
        virtual bool shouldContinue() = 0;
        virtual void run() = 0;
    };
}

#endif //VULKAN_ENGINE_RUNNABLE_HPP
