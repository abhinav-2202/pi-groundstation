#include "bits.hpp"

std::vector<uint8_t> read_bits(const std::vector<float>& mag, size_t i, size_t n)
{
    std::vector<uint8_t> bits;
    for(size_t k = 0; k < n; k++)
    {
        float first = mag[i + 16 + 2*k];
        float second = mag[i + 17 + 2*k];

        if (first > second)
        {
            bits.push_back(1);
        }
        else
        {
            bits.push_back(0);
        }
    }
    return bits;
}

uint32_t bits_to_number(const std::vector<uint8_t>& bits, size_t start, size_t count)
{
    uint32_t value = 0;
    for(size_t k = start; k < start + count; k++)
    {
        value = (value << 1) | bits[k];
    }
    return value;
}