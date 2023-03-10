#include "openings.hpp"
#include <algorithm>
#include <fstream>

[[nodiscard]] auto get_openings(const std::string &path, const bool shuffle) -> std::vector<std::string> {
    auto f = std::ifstream(path);

    if (!f.is_open()) {
        throw std::invalid_argument("Opening file not found");
    }

    auto openings = std::vector<std::string>();
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        openings.emplace_back(line);
    }

    if (shuffle) {
        std::random_shuffle(openings.begin(), openings.end());
    }

    return openings;
}
