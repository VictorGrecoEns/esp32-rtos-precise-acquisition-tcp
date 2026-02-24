import struct
from config import FRAME_SIZE

START_CMD = 1

def build_start_command(n_buffers: int, freq:int) -> bytes:
    return struct.pack('<B2H', START_CMD, n_buffers, freq)

def parse_buffer(raw_timestamp: bytes, raw_frame: bytes):
    timestamp, = struct.unpack('<Q', raw_timestamp)
    samples = struct.unpack('<' + 'H'*FRAME_SIZE, raw_frame)
    return timestamp, samples