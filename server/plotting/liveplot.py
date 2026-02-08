import threading
import numpy as np

class LivePlot:
    def __init__(self, fe : int):
        self.fe = fe
        self.buffer = []
        self.lock = threading.Lock()
        self.running = True

    def start(self):
        threading.Thread(
            target=self._run,
            daemon=True
        ).start()

    def push(self, samples):
        with self.lock:
            self.buffer.extend(samples)

    def stop(self):
        self.running = False

    def _run(self):
        import matplotlib.pyplot as plt

        plt.ion()
        fig, ax = plt.subplots(figsize=(12, 4))
        line, = ax.plot([], [], lw=1)

        ax.set_ylim(0, 4095)
        ax.grid(True)

        t0 = 0.0

        while self.running:
            with self.lock:
                if not self.buffer:
                    plt.pause(0.05)
                    continue

                data = np.array(self.buffer, dtype=np.uint16)
                self.buffer.clear()

            t  = t0 + np.arange(len(data)) / self.fe 
            t0 = t[-1] + 1 / self.fe

            line.set_data(t, data)
            ax.set_xlim(t[0], t[-1])

            plt.pause(0.01)
