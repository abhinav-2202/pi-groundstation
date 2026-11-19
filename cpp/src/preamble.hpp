#pragma once

#include <cstddef>
#include <vector>

// True if 4 pulse preamble starts at sample i
bool is_preamble(const std::vector<float>& mag, size_t i);

// Positions of every preamble candidate in the recording
std::vector<size_t> find_preambles(const std::vector<float>& mag);