#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Write number as hex text with fixed no.of digits
std::string to_hex(uint32_t value, int digits);

// Aircraft's unique 24 bit ID (bits 8-31)
std::string get_icao(const std::vector<uint8_t>& bits);

// Type code: first 5 bits of the message (bits 32-36)
uint32_t get_type_code(const std::vector<uint8_t>& bits);

// Flight callsign from type code 1-4 message (bits 40-87)
std::string get_callsign(const std::vector<uint8_t>& bits);

// Marker meaning "this message has no altitude we can decode"
const int NO_ALTITUDE = -9999;

// Barometric altitude in feet from TC 9-18 messages (bits 40-51)
int get_altitude(const std::vector<uint8_t>& bits);