#pragma once

#include <cstdint>
#include <vector>

// Signal strength (I^2 + Q^2) for every I/Q pair
std::vector<float> compute_magnitude(const std::vector<uint8_t>& raw);

// Largest value in list
float find_loudest(const std::vector<float>& mag);