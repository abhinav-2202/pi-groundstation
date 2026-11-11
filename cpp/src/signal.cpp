#include "signal.hpp"
#include <cmath>

std::vector<float> compute_magnitude(const std::vector<uint8_t>& raw)
{
    std::vector<float> mag;

    for (size_t i = 0; i + 1 < raw.size(); i += 2)
    {
        float I = raw[i] - 127.5f;
        float Q = raw[i + 1] - 127.5f;
        mag.push_back(std::sqrt(I*I + Q*Q));
    }
    return mag;
}

float find_loudest(const std::vector<float>& mag)
{
    float loudest = 0.0f;
    for (size_t i = 0; i < mag.size(); i++)
    {
        if (mag[i] > loudest)
        {
            loudest = mag[i];
        }
    }
    return loudest;
}