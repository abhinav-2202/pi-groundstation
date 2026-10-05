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