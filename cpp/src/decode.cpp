#include "decode.hpp"
#include "bits.hpp"
#include <cmath>

const double PI = 3.14159265358979;

std::string to_hex(uint32_t value, int digits)
{
    const std::string HEX_DIGITS = "0123456789ABCDEF";
    std::string text = "";

    for (int k = digits - 1; k >=0; k--)
    {
        uint32_t nibble = (value >> (4 * k)) & 0xF;
        text += HEX_DIGITS[nibble];
    }
    return text;
}

std::string get_icao(const std::vector<uint8_t>& bits)
{
    return to_hex(bits_to_number(bits, 8, 24), 6);
}

uint32_t get_type_code(const std::vector<uint8_t>& bits)
{
    return bits_to_number(bits, 32, 5);
}

std::string get_callsign(const std::vector<uint8_t>& bits)
{
    const std::string CHARSET = "#ABCDEFGHIJKLMNOPQRSTUVWXYZ##### ###############0123456789######";
    std::string callsign = "";

    for (int k = 0; k < 8; k++)
    {
        uint32_t number = bits_to_number(bits, 40 + 6 * k, 6);
        callsign += CHARSET[number];
    }
    
    while(!callsign.empty() && callsign.back() == ' ')
    {
        callsign.pop_back();
    }
    return callsign;
}

int get_altitude(const std::vector<uint8_t>& bits)
{
    if (bits[47] == 0)
    {
        return NO_ALTITUDE;
    }

    uint32_t raw = bits_to_number(bits, 40, 12);
    uint32_t left = raw >> 5;
    uint32_t right = raw & 0xF;
    uint32_t n = (left << 4) | right;

    return static_cast<int>(n) * 25 - 1000;
}

Velocity get_velocity(const std::vector<uint8_t>& bits)
{
    int east = static_cast<int>(bits_to_number(bits, 46, 10)) - 1;
    int north = static_cast<int>(bits_to_number(bits, 57, 10)) - 1;

    if (bits[45] == 1)
    {
        east = -east;
    }
    if (bits[56] == 1)
    {
        north = -north;
    }

    Velocity v;
    v.speed = std::sqrt(east * east + north * north);
    v.heading = std::atan2(east, north) * 180.0 / PI;
    if (v.heading < 0)
    {
        v.heading += 360.0;
    }
    
    int rate = (static_cast<int>(bits_to_number(bits, 69, 9)) - 1) * 64;
    if (bits[68] == 1)
    {
        rate = -rate;
    }
    v.vertical_rate = rate;

    return v;
}

CprParts get_cpr_parts(const std::vector<uint8_t>& bits)
{
    CprParts p;
    p.odd = (bits[53] == 1);
    p.lat = bits_to_number(bits, 54, 17) / 131072.0;
    p.lon = bits_to_number(bits, 71, 17) / 131072.0;

    return p;
}

int positive_mod(int a, int b)
{
    int r = a % b;
    if (r < 0)
    {
        r += b;
    }
    return r;
}

Latitudes decode_latitudes(const CprParts& even, const CprParts& odd)
{
    int j = static_cast<int>(std::floor(59 * even.lat - 60 * odd.lat + 0.5));

    Latitudes result;
    result.even = 6.0 * (positive_mod(j, 60) + even.lat);
    result.odd = (360.0 / 59.0) * (positive_mod(j, 59) + odd.lat);

    if (result.even >= 270)
    {
        result.even -= 360;
    }
    if (result.odd >= 270)
    {
        result.odd -= 360;
    }
    return result;
}

int get_nl(double lat)
{
    if (lat == 0)
    {
        return 59;
    }
    if (std::fabs(lat) == 87)
    {
        return 2;
    }
    if (std::fabs(lat) > 87)
    {
        return 1;
    }

    double a = 1 - std::cos(PI / 30);
    double b = std::cos(PI / 180 * lat);
    return static_cast<int>(std::floor(2 * PI / std::acos(1 - a / (b * b))));
}

double decode_longitude(const CprParts& even, const CprParts& odd, int nl)
{
    int k = static_cast<int>(std::floor((nl - 1) * even.lon - nl * odd.lon + 0.5));
    double lon = (360.0 / nl) * (positive_mod(k, nl) + even.lon);

    if (lon >= 180)
    {
        lon -= 360;
    }
    return lon;
}

Position decode_position(const CprParts& even, const CprParts& odd)
{
    Position pos = {};

    Latitudes lat = decode_latitudes(even, odd);
    int nl = get_nl(lat.even);
    if (nl != get_nl(lat.odd))
    {
        return pos;
    }

    pos.lat = lat.even;
    pos.lon = decode_longitude(even, odd, nl);
    pos.valid = true;
    return pos;
}