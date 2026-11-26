#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "decode.hpp"

// Everything we know about one plane : "CARD"
struct Aircraft
{
    std::string icao;
    std::string callsign;
    int altitude;

    bool has_velocity;
    Velocity velocity;

    CprParts even;
    CprParts odd;
    bool have_even;
    bool have_odd;
    Position position;

    int messages;
};

// Fresh, empty card for a plane we have seen for first time
Aircraft new_aircraft(const std::string& icao);

// Update plane's card with one valid DF17 message
void update_aircraft(Aircraft& plane, const std::vector<uint8_t>& bits);