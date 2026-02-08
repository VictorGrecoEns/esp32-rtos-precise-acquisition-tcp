import struct
from pathlib import Path
BLOC_SIZE = 256

class DataStorage:
    def __init__(self, filepath, fe):
        self.filepath = Path(filepath)
        self.fe = fe
        self._file = open(self.filepath, "wb")
        self._write_header()

    def _write_header(self):
        self._file.write(struct.pack("<I", self.fe))
        self._file.flush()

    def append_block(self, samples):
        if len(samples) != BLOC_SIZE:
            raise ValueError("Bloc de taille invalide")

        self._file.write(struct.pack(
            f"<{BLOC_SIZE}H", *samples
        ))
        self._file.flush()

    def close(self):
        self._file.close()
