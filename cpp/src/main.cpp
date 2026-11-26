#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "aircraft.hpp"
#include "bits.hpp"
#include "crc.hpp"
#include "preamble.hpp"
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
    std::vector<size_t> candidates = find_preambles(mag);

    std::map<std::string, Aircraft> table;

    for (size_t n = 0; n < candidates.size(); n++)
    {
        std::vector<uint8_t> bits = read_bits(mag, candidates[n], 112);

        if (bits_to_number(bits, 0, 5) != 17)
        {
            continue;
        }
        if (!crc_ok(bits))
        {
            continue;
        }
        process_message(table, bits);
    }

    print_table(table);
    return 0;
}