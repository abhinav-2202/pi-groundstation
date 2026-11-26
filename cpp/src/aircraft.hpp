#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

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

// Find or create plane's card in table and then update it
void process_message(std::map<std::string, Aircraft>& table, const std::vector<uint8_t>& bits);

// Print every plane in the table
void print_table(const std::map<std::string, Aircraft>& table);

// Number as text with fixed no.of decimals
std::string format_number(double value, int decimals);

// Position as text
std::string format_position(const Position& pos);