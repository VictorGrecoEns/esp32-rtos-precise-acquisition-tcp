import struct
from pathlib import Path

FILE_HEADER_FMT = "<I"  # fe (uint32)
SAMPLE_FMT = "<H" # (uint16)

class DataStorage:
    def __init__(self, filepath, block_size, fe):
        self.filepath = Path(filepath)
        self.block_size = block_size
        self._file = open(self.filepath, "wb")
        self._header_written = False
        self._fe = int(fe)
        self._write_header()

    def _write_header(self):
        header = struct.pack(FILE_HEADER_FMT, self._fe, self.block_size)
        self._file.write(header)
        self._header_written = True

    def append_block(self, samples):
        if len(samples) != self.block_size:
            raise ValueError(f"Taille du bloc invalide: attendu {self.block_size}, reçu {len(samples)}")
        for v in samples:
            if not (0 <= v <= 4095):
                raise ValueError(f"Valeur hors plage: {v}")

        # Écriture du bloc (samples uniquement, pas de timestamp)
        n = self._file.write(struct.pack(f"<{self.block_size}H", *samples))
        self._file.flush()
        print(f"Bloc écrit ! ({n} octets)")

    def close(self):
        if self._file:
            self._file.close()
