import struct
import os
from config import NB_CAPTEURS, SAMPLES_PER_CAPTEUR, DIR_PATH

RECORD_SIZE = 8 + 2*SAMPLES_PER_CAPTEUR

def reorder_files():

    for i in range(NB_CAPTEURS):
        path = os.path.join(DIR_PATH, f"ch_{i+1}.bin")

        with open(path, "rb") as f:
            content = f.read()

        records = []
        for offset in range(0, len(content), RECORD_SIZE):
            block = content[offset:offset+RECORD_SIZE]
            timestamp, = struct.unpack('<Q', block[:8])
            records.append((timestamp, block))

        records.sort(key=lambda x: x[0])

        with open(path, "wb") as f:
            for _, block in records:
                f.write(block)