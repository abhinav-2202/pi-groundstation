#include <iostream>
#include <vector>

#include "recording.hpp"
#include "signal.hpp"
#include "preamble.hpp"
#include "bits.hpp"
#include "crc.hpp"
#include "decode.hpp"

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

    // Find real ADS-B messages: DF17 and CRC pass
    std::vector<size_t> good;
    for (size_t k = 0; k < candidates.size(); k++)
    {
        std::vector<uint8_t> bits = read_bits(mag, candidates[k], 112);
        if (bits_to_number(bits, 0, 5) == 17 && crc_ok(bits))
        {
            good.push_back(candidates[k]);
        }
    }
    std::cout << "Valid ADS-B messages: " << good.size() << "\n";

    std::cout << "First 5 positions: ";
    for (size_t k = 0; k < 5 && k < good.size(); k++)
    {
        std::cout << " " << good[k];
    }
    std::cout << "\n";

    // Decode the first real message
    std::vector<uint8_t> first = read_bits(mag, good[0], 112);
    std::cout << "ICAO:      " << get_icao(first) << "\n";
    std::cout << "Type code: " << get_type_code(first) << "\n";
    std::cout << "Altitude:  " << get_altitude(first) << " ft\n";

    // Find the first identification message (TC 1-4) and print callsign
    for (size_t n = 0; n < good.size(); n++)
    {
        std::vector<uint8_t> msg = read_bits(mag, good[n], 112);
        uint32_t tc = get_type_code(msg);

        if (tc >= 1 && tc <= 4)
        {
            std::cout << "Aircraft " << get_icao(msg)
                      << " Callsign: " << get_callsign(msg) << "\n";

            break;
        }
    }

    return 0;
}