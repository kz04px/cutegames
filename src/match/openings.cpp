#include "openings.hpp"
#include <algorithm>
#include <fstream>
#include <random>

[[nodiscard]] auto get_openings(const std::string &path, const bool shuffle) -> std::vector<std::string> {
    auto file = std::ifstream(path);

    if (!file.is_open()) {
        throw std::invalid_argument("Opening file not found");
    }

    auto openings = std::vector<std::string>();
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        openings.emplace_back(line);
    }

    if (shuffle) {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(std::begin(openings), std::end(openings), g);
    }

    return openings;
}
