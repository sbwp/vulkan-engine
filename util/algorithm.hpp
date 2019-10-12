//
// Created by sabrina on 10/7/19.
//

#ifndef VULKAN_ENGINE_ALGORITHM_HPP
#define VULKAN_ENGINE_ALGORITHM_HPP

#include <algorithm>
#include <functional>

namespace Util {
    template <typename Collection, typename ValueType>
    typename Collection::iterator find(Collection collection, ValueType value) {
        return std::find(begin(collection), end(collection), value);
    }

    template <typename Collection, typename ValueType>
    bool containsValue(Collection collection, ValueType value) {
        return find(collection, value) != end(collection);
    }

    template <typename Collection, typename ValueType>
    bool containsWhere(Collection collection, std::function<bool(ValueType)> const& lambda) {
        return std::find_if(begin(collection), end(collection), lambda) != end(collection);
    }

    template <typename T>
    T const& clamp(T const& value, T const& min, T const& max) {
        return std::max(min, std::min(max, value));
    }
}

#endif //VULKAN_ENGINE_ALGORITHM_HPP
