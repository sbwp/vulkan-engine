//
// Created by sabrina on 10/18/19.
//

#include "algorithm.hpp"
#include "../logger/logger.hpp"

namespace Util {
	std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if(!file.is_open()) {
			throw Logger::error("failed to load file " + filename);
		}

		auto fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}
}