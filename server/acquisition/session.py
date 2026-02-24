from network.protocol import build_start_command, parse_buffer
from acquisition.buffer import Buffer
from storage.writer import RawWriter
from storage.reorder import reorder_files
from config import FRAME_SIZE, DIR_PATH
from os.path import getsize, join

class AcquisitionSession:

    def __init__(self, server, n_buffers, freq_acq):
        self.server = server
        self.n_buffers = n_buffers
        self.freq_acq = freq_acq
        self.writer = RawWriter()

    def run(self):

        self.server.start()

        # START ESP32
        cmd = build_start_command(self.n_buffers, self.freq_acq)
        self.server.send(cmd)

        buffers_received = 0

        try:
            while buffers_received < self.n_buffers:

                raw_ts = self.server.recv_exact(8)
                raw_frame = self.server.recv_exact(FRAME_SIZE * 2)

                timestamp, samples = parse_buffer(raw_ts, raw_frame)
                buffer = Buffer(timestamp, samples)

                self.writer.write_buffer(buffer)

                print(f"{buffers_received} - timestamp={timestamp}")
                buffers_received += 1


        finally:

            self.writer.close()
            self.server.close()
            sizeOfFiles = getsize(join(DIR_PATH, f"ch_1.bin"))
            print(f"Acquisition terminée ({1e-3 * sizeOfFiles:.2f} ko)")
            

        # Réécriture triée
        # reorder_files()
        # print("Fichiers réordonnés par timestamp")