#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>

int main()
{
    std::ifstream file("../recordings/modes1.bin", std::ios::binary);

    if (!file)
    {
        std::cout << "Could not open file\n";
        return 1;
    }

    std::vector<uint8_t> raw;
    char byte;

    while(file.get(byte))
    {
        raw.push_back(static_cast<uint8_t>(byte));
    }

    std::cout << "Numbers in file: " << raw.size() << "\n";

    std::cout << "First 20 nums: ";
    for (int i = 0; i < 20; i++)
    {
        std::cout << " " << static_cast<int>(raw[i]);
    }
    std::cout << "\n";

    return 0;
}