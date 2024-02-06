#ifndef MATCH_OPENINGS_HPP
#define MATCH_OPENINGS_HPP

#include <string>
#include <vector>

[[nodiscard]] auto get_openings(const std::string &path, const bool shuffle = false) -> std::vector<std::string>;

#endif
