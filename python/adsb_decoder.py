"""
ADS-B DECODER (Python Prototype)

Reads recording of radio signal at 1090 MHz and finds real ADS-B
messages sent by aircraft.

Steps:
    1. Load and open raw recording file and calculate signal strength
    2. Find preamble (message starting - 4 spike pattern)
    3. Read the bits after preamble to determine Downlink Format
       (Only DF 17 format is for ADS-B messages)
    4. Check each message with CRC (to filter fake messages)
    5. Check type code and decode information from the message
"""

from pathlib import Path
import numpy as np
import math

# Path to recording
HERE = Path(__file__).parent
RECORDING = HERE.parent / "recordings" / "modes1.bin"

# Standard 25 bit CRC generator for ADS-B
CRC_GENERATOR = [1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0,0,0,0,1,0,0,1]

# Message length
LONG_MESSAGE_BITS = 112

# DF number for ADS-B
DF_ADSB = 17

# Load recording and calculate signal strengths
def load_magnitude(path):
    raw = np.fromfile(path, dtype = np.uint8)
    centred = raw.astype(np.float32) - 127.5        # setting 127.5 as zero
    I = centred[0::2]
    Q = centred[1::2]
    return np.sqrt(I**2 + Q**2)                     # magnitude of signal (I and Q components)

# Find Preambles
# Preamble - In 10 bits -> 0,2,7,9 bits are high -> that is a preamble
def is_preamble(mag, i):
    highs = [mag[i], mag[i+2], mag[i+7], mag[i+9]]
    lows = [mag[i+1], mag[i+3], mag[i+4], mag[i+5], mag[i+6], mag[i+8]]

    return min(highs) > max(lows)                   # every spike must be stronger than gaps

def find_preambles(mag):
    found = []
    for i in range(len(mag) - 250):                 # -250 as we read upto 240 samples from start so avoiding reading after file ends and crashing
        if is_preamble(mag,i):
            found.append(i)
    return found

# Read bits
# Reading bits after 16 samples after preamble start
# Pulse Position Modulation - first sample bigger = 1. second sample bigger = 0
def read_bits(mag, i, n):
    bits = []
    for k in range(n):
        first = mag[i + 16 + 2*k]
        second = mag[i + 17 + 2*k]
        if first > second:
            bits.append(1)
        else:
            bits.append(0)
    return bits

# Convert list of binary bits into int
def bits_to_number(bits):
    value = 0
    for b in bits:
        value = value * 2 + b
    return value

# Convert list of bits into hex string
def bits_to_hex(bits):
    return hex(bits_to_number(bits))[2:].upper()

# CRC check
# Long division using XOR and returns remainder bits
def crc_remainder(bits, gen):
    b = list(bits)
    data_len = len(b) - (len(gen) - 1)              # 112 - 24 = 88 bits

    for i in range(data_len):
        if b[i] == 1:
            for j in range(len(gen)):
                b[i+j] = b[i+j] ^ gen[j]

    return b[data_len:]                             # last 24 bits = remainder

# Returns true if messages pass CRC (remainder is all zeros)
def crc_ok(bits):
    return sum(crc_remainder(bits, CRC_GENERATOR)) == 0

# Decode the main message contents
# Aircraft's unique 24 bit ID (bits 8-31) as hex characters
def get_icao(bits):
    number = bits_to_number(bits[8:32])
    return format(number, "06X")

# Type code - first 5 bits of the main data (bits 32-36)
def get_type_code(bits):
    return bits_to_number(bits[32:37]) 

# Decoding callsign message (type code = 1 to 4)
# Lookup table - character at position n is the meaning of number n
CHARSET = "#ABCDEFGHIJKLMNOPQRSTUVWXYZ##### ###############0123456789######"

def get_callsign(bits):
    callsign = ""
    for k in range(8):
        start = 40 + 6*k
        number = bits_to_number(bits[start:start + 6])
        callsign = callsign + CHARSET[number]
    return callsign.strip()                         # .strip() removes extra spaces added at the end

# Decoding altitude in feet (type code = 9 to 18)
def get_altitude(bits):
    alt_bits = bits[40:52]                          # 12 altitude bits
    q_bit = alt_bits[7]

    if q_bit == 0:
        return None                                 # old coding system - we skip this method

    n_bits = alt_bits[:7] + alt_bits[8:]            # remove q bit - 11 bits
    n = bits_to_number(n_bits)
    return 25*n - 1000                              # -1000 to handle airports below sealevel

# Decoding velocity in knots (type code = 19)
def get_velocity_parts(bits):
    ew_dir = bits[45];                              # 0 = east ; 1 = west
    ew_speed = bits_to_number(bits[46:56]) - 1      # -1 is for 0th bit

    ns_dir = bits[56];
    ns_speed = bits_to_number(bits[57:67]) - 1

    if ew_dir == 1:
        v_east = -ew_speed
    else:
        v_east = ew_speed

    if ns_dir == 1:
        v_north = -ns_speed
    else:
        v_north = ns_speed

    return v_east, v_north

# Decoding exact speed and direction using velocity parts
# Ground speed in knots and heading in degrees from North
def get_speed_and_heading(bits):
    v_east, v_north = get_velocity_parts(bits)
    speed = math.sqrt(v_east**2 + v_north**2)

    heading = math.degrees(math.atan2(v_east, v_north))
    heading = heading % 360

    return speed, heading

# Decoding vertical rate
# Ascend (+) or Descent (-) rate in feet per minute (steps of 64)
def get_vertical_rate(bits):
    direction = bits[68]                            # 0 = ascend ; 1 = descent
    number = bits_to_number(bits[69:78])            # 9 bits
    rate = (number - 1) * 64

    if direction == 1:
        rate = -rate

    return rate

# Main Program
def main():
    # 1. Load and calculate magnitudes
    mag = load_magnitude(RECORDING)
    print("Samples in recording: ", len(mag))

    # 2. Find all preambles
    candidates = find_preambles(mag)
    print("Preamble candidates: ", len(candidates))

    # 3. Keep only ADS-B (DF 17) candidates
    adsb = []
    for i in candidates:
        df = bits_to_number(read_bits(mag, i, 5))
        if df == DF_ADSB:
            adsb.append(i)
    print("DF 17 candidates: ", len(adsb))

    # 4. Keep only messages that pass the CRC
    good = []
    for i in adsb:
        bits = read_bits(mag, i, LONG_MESSAGE_BITS)
        if crc_ok(bits):
            good.append(i)
    print("Passed CRC (real ADS-B messages): ", len(good))

    # 5. Printing first 5 real messages
    print()
    print("First 5 messages:")
    for i in good[:5]:
        bits = read_bits(mag, i, LONG_MESSAGE_BITS)
        print(" sample", i, ":", bits_to_hex(bits))

    # 6. Split message into its parts
    print()
    bits = read_bits(mag, good[0], LONG_MESSAGE_BITS)
    print("Message:   ", bits_to_hex(bits))
    print("DF:        ", bits_to_number(bits[0:5]))
    print("ICAO:      ", get_icao(bits))
    print("Type code: ", get_type_code(bits))

    # 7. Count the type codes across all real messages
    print()
    tc_counts = {}
    for i in good:
        bits = read_bits(mag, i, LONG_MESSAGE_BITS)
        tc = get_type_code(bits)
        tc_counts[tc] = tc_counts.get(tc, 0) + 1    # storing in dictionary as key and value pairs

    for tc in sorted(tc_counts):
        print("Type code ", tc, ":", tc_counts[tc], " messages")

    # 8a. Testing callsign decoder with existing example
    print()
    example = "8D4840D6202CC371C32CE0576098"
    example_bits = [int(c) for c in format(int(example, 16), "0112b")]
    print("Example callsign: ", get_callsign(example_bits))

    # 8b. Find callsign in our recording
    for i in good:
        bits = read_bits(mag, i, LONG_MESSAGE_BITS)
        tc = get_type_code(bits)
        if 1 <= tc <= 4:
            print("Aircraft", get_icao(bits), "Callsign: ", get_callsign(bits))
            break

    # 9. Altitude from every position message
    print()
    altitudes = []
    for i in good:
        bits = read_bits(mag, i, LONG_MESSAGE_BITS)
        tc = get_type_code(bits)
        if 9 <= tc <= 18:
            altitudes.append(get_altitude(bits))

    print("Position messages: ", len(altitudes))
    print("First altitude: ", altitudes[0], " ft")
    print("Last altitude: ", altitudes[-1], " ft")

    # 10. Velocity : Speed, Heading and Vertical rate
    print()
    for i in good:
        bits = read_bits(mag, i, LONG_MESSAGE_BITS)
        if get_type_code(bits) == 19:
            speed, heading = get_speed_and_heading(bits)
            vrate = get_vertical_rate(bits)
            print("Speed: ", round(speed), "knots   Heading: ", round(heading,1),
                  "degrees  Vertical rate: ", vrate, "ft/min")
            break

# Run main() only when this file is run directly
if __name__ == "__main__":
    main()