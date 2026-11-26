#include "aircraft.hpp"

#include <iostream>
#include <cmath>
#include<iomanip>
#include <sstream>

Aircraft new_aircraft(const std::string& icao)
{
    Aircraft plane = {};
    plane.icao = icao;
    plane.altitude = NO_ALTITUDE;
    return plane;
}

void update_aircraft(Aircraft& plane, const std::vector<uint8_t>& bits)
{
    plane.messages++;
    uint32_t tc = get_type_code(bits);

    if (tc >= 1 && tc <= 4)
    {
        plane.callsign = get_callsign(bits);
    }
    else if (tc >= 9 && tc <= 18)
    {
        int alt = get_altitude(bits);
        if (alt != NO_ALTITUDE)
        {
            plane.altitude = alt;
        }

        CprParts p = get_cpr_parts(bits);
        if (p.odd)
        {
            plane.odd = p;
            plane.have_odd = true;
        }
        else
        {
            plane.even = p;
            plane.have_even = true;
        }

        if (plane.have_even && plane.have_odd)
        {
            Position pos = decode_position(plane.even, plane.odd);
            if (pos.valid)
            {
                plane.position = pos;
            }
        }
    }
    else if (tc == 19)
    {
        plane.velocity = get_velocity(bits);
        plane.has_velocity = true;
    }
}

void process_message(std::map<std::string, Aircraft>& table, const std::vector<uint8_t>& bits)
{
    std::string icao = get_icao(bits);

    if (table.count(icao) == 0)
    {
        table[icao] = new_aircraft(icao);
    }
    update_aircraft(table[icao], bits);
}

std::string format_number(double value, int decimals)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(decimals) << value;
    return out.str();
}

std::string format_position(const Position& pos)
{
    std::string ns = "N";
    std::string ew = "E";
    if (pos.lat < 0)
    {
        ns = "S";
    }
    if (pos.lon < 0)
    {
        ew = "W";
    }

    return format_number(std::fabs(pos.lat), 4) + "° " + ns + ", "
         + format_number(std::fabs(pos.lon), 4) + "° " + ew;
}

void print_table(const std::map<std::string, Aircraft>& table)
{
    std::cout << std::left
              << std::setw(8)  << "ICAO"
              << std::setw(10) << "Callsign"
              << std::right
              << std::setw(8)  << "Alt(ft)"
              << std::setw(8)  << "Spd(kt)"
              << std::setw(7)  << "Hdg"
              << std::setw(8)  << "V/S"
              << std::setw(6)  << "Msgs"
              << "  Position\n";
    
    for (const auto& entry : table)
    {
        const Aircraft& plane = entry.second;

        std::string callsign = "-";
        std::string altitude = "-";
        std::string speed    = "-";
        std::string heading  = "-";
        std::string vs       = "-";
        std::string position = "-";

        if (!plane.callsign.empty())
        {
            callsign = plane.callsign;
        }
        if (plane.altitude != NO_ALTITUDE)
        {
            altitude = std::to_string(plane.altitude);
        }
        if (plane.has_velocity)
        {
            speed = format_number(plane.velocity.speed, 0);
            heading = format_number(plane.velocity.heading, 1);
            vs = std::to_string(plane.velocity.vertical_rate);
        }
        if (plane.position.valid)
        {
            position = format_position(plane.position);
        }

        std::cout << std::left
                  << std::setw(8)  << plane.icao
                  << std::setw(10) << callsign
                  << std::right
                  << std::setw(8)  << altitude
                  << std::setw(8)  << speed
                  << std::setw(7)  << heading
                  << std::setw(8)  << vs
                  << std::setw(6)  << plane.messages
                  << "  " << position << "\n";
    }

    std::cout << "\nAircraft seen: " << table.size() << "\n";
}