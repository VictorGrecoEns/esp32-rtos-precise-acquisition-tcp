import sys
import time
import numpy as np
from config import SAMPLES_PER_CAPTEUR, NB_CAPTEURS


class TerminalTable:

    def __init__(self, n_buffers, freq, n_lines=10):
        self.n_buffers = n_buffers
        self.freq = freq
        self.n_lines = n_lines

        self.last_timestamp = None
        self.last_recv_time = None

        self.tfill_theoretical = SAMPLES_PER_CAPTEUR / freq

        self.emptyLine = "|   ---  |" + NB_CAPTEURS * "   ------  |" + "    -------- |   --------  |"
        self.totalLine = "-" * (10 + NB_CAPTEURS * 13 + 26)
        self._print_header()


    def _print_header(self):

        print(self.totalLine)

        mean_header = ""
        for i in range(NB_CAPTEURS):
            mean_header += f"  Mean CH{i+1} |"

        print(f"| Buffer |{mean_header} Δt WiFi (s) | fe eff (Hz) |")

        print(self.totalLine)

        

        print(self.emptyLine + (self.n_lines - 1) * f"\n{self.emptyLine}")

        print(self.totalLine)

        print(2 * "\n")

        sys.stdout.write(f"\033[4F")
        sys.stdout.flush()


    def update(self, index, timestamp, sensors):

        now = time.perf_counter()

        # ===== Moyennes =====
        means = [float(np.mean(sensors[i])) for i in range(NB_CAPTEURS)]

        # ===== Δt WiFi =====
        if self.last_recv_time is None:
            delta_wifi = 0.0
        else:
            delta_wifi = now - self.last_recv_time
        self.last_recv_time = now

        # ===== fe effective =====
        if self.last_timestamp is None:
            fe_eff = 0.0
        else:
            delta_acq = (timestamp - self.last_timestamp) * 1e-6
            fe_eff = SAMPLES_PER_CAPTEUR / delta_acq if delta_acq > 0 else 0.0
        self.last_timestamp = timestamp

        # ===== Remontée bloc =====
        if (index - 1) % self.n_lines == 0:
            sys.stdout.write(f"\033[{self.n_lines}F")
            sys.stdout.flush()
        # ===== Ligne affichée =====
        line = (
            f"| {index:6d} |"
        )

        for m in means:
            line += f"{m:10.2f} |"

        line += f"  {delta_wifi:10.4f} |{fe_eff:12.2f} |"

        print(line)

        # ===== Fin acquisition =====
        if index == self.n_buffers:
            if index % self.n_lines != 0:
                print((self.n_lines-1 - ((index-1) % self.n_lines)-1) * f"{self.emptyLine}\n"+self.emptyLine)
            print(self.totalLine)
        