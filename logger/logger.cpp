//
// Created by sabrina on 10/7/19.
//

#include "logger.hpp"

namespace Logger {
    void assertTrue(bool condition, std::string const& errorMessage) {
        if (!condition) {
            throw error(errorMessage);
        }
    }

    void assertFalse(bool condition, std::string const& errorMessage) {
        if (condition) {
            throw error(errorMessage);
        }
    }
}