import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

HERE = Path(__file__).parent
FILE = HERE.parent / "recordings" / "modes1.bin"

raw = np.fromfile(FILE, dtype=np.uint8)

print("Numbers in file = ", len(raw))
print("First 20 numbers = ", raw[:20])

centred = raw.astype(np.float32) - 127.5            # Treating 127.5 as centre of the range of 0-255
print("First 20 centred values = ", centred[:20])

I = centred[0::2]   # Even indexed values
Q = centred[1::2]   # Odd indexed values

print("No.of I values = ", len(I))
print("No.of Q values = ", len(Q))
print("First 5 I = ", I[:5])
print("First 5 Q = ", Q[:5])

mag = np.sqrt(I**2 + Q**2)   # Magnitude of signal

print("First 10 magnitudes = ", mag[:10])
print("Max magnitude = ", mag.max())
print("Min magnitude = ", mag.min())
print("Typical magnitude = ", np.median(mag))

start = 11513            # Zooming into one message
end = 11773
range = mag[start:end]

plt.plot(range, marker = ".")
plt.title("One aircraft message")
plt.xlabel("Sample number")
plt.ylabel("Magnitude")
plt.grid(True)
plt.show()