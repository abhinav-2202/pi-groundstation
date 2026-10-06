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
"""

from pathlib import Path
import numpy as np

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

# Run main() only when this file is run directly
if __name__ == "__main__":
    main()