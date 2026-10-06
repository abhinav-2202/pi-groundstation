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