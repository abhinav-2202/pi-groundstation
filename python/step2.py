import numpy as np
from pathlib import Path

HERE = Path(__file__).parent
FILE = HERE.parent / "recordings" / "modes1.bin"

raw = np.fromfile(FILE, dtype=np.uint8)
centred = raw.astype(np.float32) - 127.5            # Treating 127.5 as centre of the range of 0-255

I = centred[0::2]   # Even indexed values
Q = centred[1::2]   # Odd indexed values

mag = np.sqrt(I**2 + Q**2)   # Magnitude of signal

start = 11523            # Zooming into one message

for k in range(10):
    print(k, mag[start + k])

def is_preamble(mag, i):
    highs = [mag[i], mag[i+2], mag[i+7], mag[i+9]]
    lows = [mag[i+1], mag[i+3], mag[i+4], mag[i+5], mag[i+6], mag[i+8]]

    if min(highs) > max(lows):
        return True
    else:
        return False

found = []

for i in range(len(mag) - 10):      # -10 so that program won't look after file ends and crashes
    if is_preamble(mag,i):
        found.append(i)

print("Preambles found: ", len(found))
print("First 10 positions: ", found[:10])