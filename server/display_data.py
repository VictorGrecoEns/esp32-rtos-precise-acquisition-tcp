import struct
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
import os
from config import parse_args, make_repo, NB_CAPTEURS, DATA_NAME

args = parse_args()
DATA_PATH = make_repo(args.repo, createRepo=False)

def extract_data(folder):
    all_sensors = []
    freq = 0

    for i in range(NB_CAPTEURS):
        filepath = Path(folder) / f"ch_{i+1}.bin"
        sensor_data = []

        with open(filepath, "rb") as f:
            # --- Header ---
            freq_i, n_buffer, samples_per_buffer = struct.unpack("<3I", f.read(12))
            freq = freq_i  # identique pour tous les fichiers

            # --- Lecture des buffers ---
            for _ in range(n_buffer):
                raw = f.read(8 + samples_per_buffer * 2)
                unpacked = struct.unpack('<Q' + 'H' * samples_per_buffer, raw)

                timestamp = unpacked[0]
                samples = unpacked[1:]

                sensor_data.extend(samples)

        all_sensors.append(np.array(sensor_data, dtype=np.uint16))

    # --- Création du vecteur temps ---
    n_total_samples = len(all_sensors[0])
    times = np.arange(n_total_samples) / freq

    return times, all_sensors


t, sensors = extract_data(DATA_PATH)

# --- Plot ---
fig, axs = plt.subplots(NB_CAPTEURS, 1, figsize=(12, 6), sharex=True)

if NB_CAPTEURS == 1:
    axs = [axs]

for i in range(NB_CAPTEURS):
    axs[i].plot(t, sensors[i], linewidth=1)
    axs[i].grid(True)
    axs[i].set_ylabel(f"Ch{i+1} (0-4095)")

axs[-1].set_xlabel("Temps (s)")
plt.tight_layout()
plt.show()