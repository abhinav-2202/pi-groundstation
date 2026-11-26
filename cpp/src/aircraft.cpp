#include "aircraft.hpp"

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