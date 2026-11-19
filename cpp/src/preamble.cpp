#include "preamble.hpp"
#include <algorithm>

bool is_preamble(const std::vector<float>& mag, size_t i)
{
    float smallest_high = std::min({mag[i], mag[i + 2], mag[i + 7], mag[i + 9]});
    float biggest_low = std::max({mag[i + 1], mag[i + 3], mag[i + 4], mag[i + 5], mag[i + 6], mag[i + 8]});

    return smallest_high > biggest_low;
}

std::vector<size_t> find_preambles(const std::vector<float>& mag)
{
    std::vector<size_t> found;
    if (mag.size() < 250)       // as size_t is unsigned so not checking this may lead to wrap around bugs
    {
        return found;
    }

    for (size_t i = 0; i < mag.size() - 250; i++)
    {
        if (is_preamble(mag, i))
        {
            found.push_back(i);
        }
    }
    return found;
}