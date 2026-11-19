#include <iostream>
#include <vector>

#include "recording.hpp"
#include "signal.hpp"
#include "preamble.hpp"
#include "bits.hpp"

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
    std::vector<size_t> candidates = find_preambles(mag);
    
    std::cout << "Numbers in file:     " << raw.size() << "\n";
    std::cout << "Samples:             " << mag.size() << "\n";
    std::cout << "Loudest:             " << find_loudest(mag) << "\n";
    std::cout << "Preamble candidates: " << candidates.size() << "\n";

    std::cout << "First 10 positions: ";
    for (size_t k = 0; k < 10 && k < candidates.size(); k++)
    {
        std::cout << " " << candidates[k];
    }
    std::cout << "\n";

    // Check: message types at our known positions
    std::cout << "DF at 11523: " << bits_to_number(read_bits(mag, 11523, 5), 0, 5) << "\n";
    std::cout << "DF at 11683: " << bits_to_number(read_bits(mag, 11683, 5), 0, 5) << "\n";

    // Count how many candidates are DF 17 (ADS-B)
    int df17 = 0;
    for (size_t k = 0; k < candidates.size(); k++)
    {
        std::vector<uint8_t> bits = read_bits(mag, candidates[k], 5);
        if(bits_to_number(bits, 0, 5) == 17)
        {
            df17++;
        }
    }
    std::cout << "DF 17 candidates: " << df17 << "\n";

    return 0;
}