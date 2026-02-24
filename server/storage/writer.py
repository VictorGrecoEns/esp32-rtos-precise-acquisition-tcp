import struct
import os
from config import NB_CAPTEURS, SAMPLES_PER_CAPTEUR, DIR_PATH

class RawWriter:

    def __init__(self):
        self.files = [
            open(os.path.join(DIR_PATH, f"ch_{i+1}.bin"), "wb")
            for i in range(NB_CAPTEURS)
        ]

    def write_buffer(self, buffer):
        sensors = buffer.split_by_sensor()

        for i, cap_samples in enumerate(sensors):
            data = [buffer.timestamp]
            data.extend(cap_samples)
            packed = struct.pack('<Q' + 'H'*SAMPLES_PER_CAPTEUR, *data)
            self.files[i].write(packed)

    def close(self):
        for f in self.files:
            f.close()