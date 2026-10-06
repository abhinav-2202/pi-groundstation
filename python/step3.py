import numpy as np
from pathlib import Path

HERE = Path(__file__).parent
FILE = HERE.parent / "recordings" / "modes1.bin"

raw = np.fromfile(FILE, dtype=np.uint8)
centred = raw.astype(np.float32) - 127.5            # Treating 127.5 as centre of the range of 0-255

I = centred[0::2]   # Even indexed values
Q = centred[1::2]   # Odd indexed values

mag = np.sqrt(I**2 + Q**2)   # Magnitude of signal

def is_preamble(mag, i):
    highs = [mag[i], mag[i+2], mag[i+7], mag[i+9]]
    lows = [mag[i+1], mag[i+3], mag[i+4], mag[i+5], mag[i+6], mag[i+8]]

    if min(highs) > max(lows):
        return True
    else:
        return False

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

start = 11523            # Zooming into one message

for k in range(4):
    first = mag[start + 16 + 2*k]
    second = mag[start + 17 + 2*k]
    print("bit", k, ": first =", first, " second =", second)

print("First 8 bits:", read_bits(mag, start, 8))

def bits_to_number(bits):
    value = 0
    for b in bits:
        value = value * 2 + b
    return value

found = []

for i in range(len(mag) - 250):      # -250 as we read upto 240 samples from start so avoiding reading after file ends and crashing
    if is_preamble(mag,i):
        found.append(i)

counts = {}
for i in found:
    df = bits_to_number(read_bits(mag, i, 5))
    counts[df] = counts.get(df, 0) + 1          # Get the current count for this DF (or 0 if not yet found) and add 1

for df in sorted(counts):
    print("DF", df, ":", counts[df])

def remainder(bits, gen):
    b = list(bits)
    data_len = len(b) - (len(gen) - 1)

    for i in range(data_len):
        if b[i] == 1:
            for j in range(len(gen)):
                b[i+j] = b[i+j] ^ gen[j]

    return b[data_len:]

def hex_to_bits(hex_text):
    bits = []
    for c in hex_text:
        value = int(c, 16)              # converts hex to int
        four = format(value, "04b")     # writes int as 4 binary digits
        for ch in four:
            bits.append(int(ch))
    return bits

REAL_GEN = [1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0,0,0,0,1,0,0,1]

msg = hex_to_bits("8D4840D6202CC371C32CE0576098")
print("No.of bits: ", len(msg))
print("Remainder: ", remainder(msg, REAL_GEN))

msg[50] = 1 - msg[50]
print("flipping bit 50: ", remainder(msg, REAL_GEN))

def crc_ok(bits):
    r = remainder(bits, REAL_GEN)
    return sum(r) == 0

adsb = []
for i in found:
    df = bits_to_number(read_bits(mag, i, 5))
    if df == 17:
        adsb.append(i)

print("DF 17 messages: ", len(adsb))

good = []
for i in adsb:
    bits = read_bits(mag, i , 112)
    if crc_ok(bits):
        good.append(i)

print("Passed CRC: ", len(good))

for i in good[:5]:
    number = bits_to_number(read_bits(mag, i, 112))
    print(i, hex(number)[2:].upper())