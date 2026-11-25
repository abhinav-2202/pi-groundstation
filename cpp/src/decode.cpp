#include "decode.hpp"
#include "bits.hpp"

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