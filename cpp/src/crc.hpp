#pragma once

#include <cstdint>
#include <vector>

// True if a 112-bit message passes the Mode S CRC (zero remainder)
bool crc_ok(std::vector<uint8_t> bits);