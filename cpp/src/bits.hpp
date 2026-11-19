#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

// Read n bits from message that starts at sample i
// Each bit uses 2 samples: 1st bigger = 1, 2nd bigger = 0
std::vector<uint8_t> read_bits(const std::vector<float>& mag, size_t i, size_t n);

// Turn bits into a number. binary to integer
uint32_t bits_to_number(const std::vector<uint8_t>& bits, size_t start, size_t count);
