#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cmath>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: ./adsb <recording file>\n";
        return 1;
    }
    
    std::ifstream file(argv[1], std::ios::binary);

    if (!file)
    {
        std::cout << "Could not open " << argv[1] << "\n";
        return 1;
    }

    std::vector<uint8_t> raw;
    char byte;

    while(file.get(byte))
    {
        raw.push_back(static_cast<uint8_t>(byte));
    }

    std::cout << "Numbers in file: " << raw.size() << "\n";

    std::vector<float> mag;

    for (size_t i = 0; i + 1 < raw.size(); i += 2)
    {
        float I = raw[i] - 127.5f;
        float Q = raw[i+1] - 127.5f;
        mag.push_back(std::sqrt(I * I + Q * Q));
    }

    std::cout << "Samples: " << mag.size() << "\n";
    std::cout << "First 10 strengths:";
    for (int i = 0; i < 10; i++)
    {
        std::cout << " " << mag[i];
    }
    std::cout << "\n";

    float loudest = 0.0f;
    for (size_t i = 0; i < mag.size(); i++)
    {
        if (mag[i] > loudest)
        {
            loudest = mag[i];
        }
    }
    std::cout << "Loudest: " << loudest << "\n";

    return 0;
}