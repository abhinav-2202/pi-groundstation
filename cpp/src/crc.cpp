#include "crc.hpp"

// Fixed 25-bit CRC generator. Standard for ADS-B
const uint8_t GENERATOR[25] = {1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0,0,0,0,1,0,0,1};

bool crc_ok(std::vector<uint8_t> bits)
{
    // Long division with XOR
    for (size_t i = 0; i < 88; i++)
    {
        if (bits[i] == 1)
        {
            for (size_t j = 0; j < 25; j++)
            {
                bits[i + j] = bits[i + j] ^ GENERATOR[j];
            }
        }
    }

    // Last 24 bits are the remainder. All zeros means valid message
    for (size_t i = 88; i < 112; i++)
    {
        if (bits[i] != 0)
        {
            return false;
        }
    }
    return true;
}