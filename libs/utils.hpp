#ifndef UTIL_HPP
#define UTIL_HPP

#include <algorithm>
#include <string_view>
#include <vector>

namespace utils {

[[nodiscard]] static auto split(const std::string_view str,
                                const std::string_view delims = " ") -> std::vector<std::string_view> {
    std::vector<std::string_view> output;

    for (auto first = str.data(), second = str.data(), last = first + str.size(); second != last && first != last;
         first = second + 1) {
        second = std::find_first_of(first, last, std::cbegin(delims), std::cend(delims));

        if (first != second) {
            output.emplace_back(first, second - first);
        }
    }

    return output;
}

}  // namespace utils

#endif
