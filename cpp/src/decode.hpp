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

// Speed (knots) and heading (degrees, 0 = north, clockwise)
// Vertical rate (ft/min, negative = descending)
struct Velocity
{
    double speed;
    double heading;
    int vertical_rate;
};

// Speed and heading from TC 19 message
Velocity get_velocity(const std::vector<uint8_t>& bits);

// The three CPR pieces from a position message (TC 9-18)
struct CprParts
{
    bool odd;       // true = odd message, false = even
    double lat;     // latitude fraction, 0 to 1
    double lon;     // longitude fraction, 0 to 1
};

// Read the odd flag and the two 17-bit CPR fractions
CprParts get_cpr_parts(const std::vector<uint8_t>& bits);

// Clock-style reminder: always 0 to b-1, even when a is negative
int positive_mod(int a, int b);

// The two latitude answers, one from each grid
struct Latitudes
{
    double even;
    double odd;
};

// Latitude in degrees from one even and one odd message
Latitudes decode_latitudes(const CprParts& even, const CprParts& odd);

// No.of longitude zones at this latitude (1 to 59)
int get_nl(double lat);

// Longitude in degrees from one even and one odd message, using the even grid
double decode_longitude(const CprParts& even, const CprParts& odd, int nl);

// Decoded position. valid = false means "no answer"
struct Position
{
    bool valid;
    double lat;
    double lon;
};

// Full position from one even meesage and one odd message (checks if NLs match)
Position decode_position(const CprParts& even, const CprParts& odd);