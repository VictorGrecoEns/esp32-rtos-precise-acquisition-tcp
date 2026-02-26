import struct
from os.path import getsize, join
from config import NB_CAPTEURS, SAMPLES_PER_CAPTEUR

class RawWriter:

    def __init__(self, repo):
        self.filePath = repo        
        self.files = [
            open(join(self.filePath, f"ch_{i+1}.bin"), "wb")
            for i in range(NB_CAPTEURS)
        ]

    def write_buffer(self, buffer):
        sensors = buffer.split_by_sensor()

        for i, cap_samples in enumerate(sensors):
            data = [buffer.timestamp]
            data.extend(cap_samples)
            packed = struct.pack('<Q' + 'H'*SAMPLES_PER_CAPTEUR, *data)
            self.files[i].write(packed)

    def write_header(self, freq, n_buffer):
        packed = struct.pack('<3I',freq, n_buffer,SAMPLES_PER_CAPTEUR)
        for i in range(NB_CAPTEURS):
            self.files[i].write(packed)
        
    def get_file_size(self):
        return getsize(join(self.filePath, f"ch_1.bin"))
        
    def close(self):
        for f in self.files:
            f.close()