#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Read a whole recording into list of bytes (0-255)
std::vector<uint8_t> load_recording(const std::string& path);