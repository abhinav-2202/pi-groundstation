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