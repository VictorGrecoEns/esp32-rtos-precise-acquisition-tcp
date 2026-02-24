from network.protocol import build_start_command, parse_buffer
from acquisition.buffer import Buffer
from storage.writer import RawWriter
from storage.reorder import reorder_files
from config import FRAME_SIZE
from acquisition.terminal_table import TerminalTable

class AcquisitionSession:

    def __init__(self, server, n_buffers, freq_acq, repo):
        self.server = server
        self.n_buffers = n_buffers
        self.freq_acq = freq_acq
        self.writer = RawWriter(repo)
        self.repo = repo

    def run(self):

        self.server.start()
        table = TerminalTable(self.n_buffers, self.freq_acq)
        
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

                sensors = buffer.split_by_sensor()
                buffers_received += 1
                table.update(buffers_received, timestamp, sensors)
                


        finally:

            sizeOfFiles = self.writer.get_file_size()
            self.writer.close()
            self.server.close()
            print(f"\nAcquisition terminée ({1e-3 * sizeOfFiles:.2f} ko)")
            

        # Réécriture triée
        # reorder_files(dirname(__file__) + "/data/" + self.repo )
        # print("Fichiers réordonnés par timestamp")