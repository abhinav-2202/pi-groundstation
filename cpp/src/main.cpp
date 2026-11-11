#include <iostream>
#include <vector>

#include "recording.hpp"
#include "signal.hpp"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: ./adsb <recording file>\n";
        return 1;
    }

    std::vector<uint8_t> raw = load_recording(argv[1]);
    if (raw.empty())
    {
        std::cout << "Could not open " << argv[1] << "\n";
        return 1;
    }

    std::vector<float> mag = compute_magnitude(raw);
    
    std::cout << "Numbers in file: " << raw.size() << "\n";
    std::cout << "Samples:         " << mag.size() << "\n";
    std::cout << "Loudest:         " << find_loudest(mag) << "\n";

    return 0;
}