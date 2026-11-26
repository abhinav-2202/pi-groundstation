#include <iostream>
#include <vector>

#include "recording.hpp"
#include "signal.hpp"
#include "preamble.hpp"
#include "bits.hpp"
#include "crc.hpp"
#include "decode.hpp"
#include "aircraft.hpp"

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

    // Find the first velocity message (TC 19) and print speed and heading
    for (size_t n = 0; n < good.size(); n++)
    {
        std::vector<uint8_t> msg = read_bits(mag, good[n], 112);

        if (get_type_code(msg) == 19)
        {
            Velocity v = get_velocity(msg);
            std::cout << "Speed:   " << v.speed << " knots\n";
            std::cout << "Heading: " << v.heading << " deg\n";
            std::cout << "V/S:     " << v.vertical_rate << " ft/min\n";
            break;
        }
    }

    // Follow the plane through the whole recording
    CprParts even = {};
    CprParts odd = {};
    bool have_even = false;
    bool have_odd = false;

    int count = 0;
    Position first_pos = {};
    Position last_pos = {};

    for (size_t n = 0; n < good.size(); n++)
    {
        std::vector<uint8_t> msg = read_bits(mag, good[n], 112);
        uint32_t tc = get_type_code(msg);

        if (tc < 9 || tc > 18)
        {
            continue;
        }

        CprParts p = get_cpr_parts(msg);
        if (p.odd)
        {
            odd = p;
            have_odd = true;
        }
        else
        {
            even = p;
            have_even = true;
        }

        if (!have_even || !have_odd)
        {
            continue;
        }

        Position pos = decode_position(even, odd);
        if (!pos.valid)
        {
            continue;
        }

        if (count == 0)
        {
            first_pos = pos;
        }
        last_pos = pos;
        count++;
    }

    std::cout << "Positions decoded: " << count << "\n";
    std::cout << "First position:    " << first_pos.lat << ", " << first_pos.lon << "\n";
    std::cout << "Last position:    " << last_pos.lat << ", " << last_pos.lon << "\n";
     
    // Build one card for our aircraft from every good message
    Aircraft plane = new_aircraft(get_icao(first));
    for (size_t n = 0; n < good.size(); n++)
    {
        std::vector<uint8_t> msg = read_bits(mag, good[n], 112);
        if (get_icao(msg) == plane.icao)
        {
            update_aircraft(plane, msg);
        }
    }

    std::cout << "\nCard for " << plane.icao << "\n";
    std::cout << "  Callsign:  " << plane.callsign << "\n";
    std::cout << "  Altitude:  " << plane.altitude << " ft\n";
    std::cout << "  Speed:     " << plane.velocity.speed << " kt\n";
    std::cout << "  Heading:   " << plane.velocity.heading << " deg\n";
    std::cout << "  V/S:       " << plane.velocity.vertical_rate << " ft/min\n";
    std::cout << "  Position:  " << plane.position.lat << ", " << plane.position.lon << "\n";
    std::cout << "  Messages:  " << plane.messages << "\n";
    
    return 0;
}