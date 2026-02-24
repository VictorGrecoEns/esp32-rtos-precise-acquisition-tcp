from config import NB_CAPTEURS
class Buffer:

    def __init__(self, timestamp: int, samples: tuple):
        self.timestamp = timestamp
        self.samples = samples

    def split_by_sensor(self):
        sensors = []
        for cap in range(NB_CAPTEURS):
            cap_samples = self.samples[cap::NB_CAPTEURS]
            sensors.append(cap_samples)
        return sensors