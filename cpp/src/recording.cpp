#include "recording.hpp"
#include <fstream>

std::vector<uint8_t> load_recording(const std::string& path)
{
    std::vector<uint8_t> raw;

    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        return raw;
    }

    char byte;
    while (file.get(byte))
    {
        raw.push_back(static_cast<uint8_t>(byte));
    }
    return raw;
}