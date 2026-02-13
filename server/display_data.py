import struct
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
import os
from acquisition.controller import NB_CHANNELS
# Chemin du fichier binaire
FILENAME = os.path.dirname(__file__) + "/data/vg10022026_acq_1"

def extract_data(filename):
    # --- Lecture du fichier ---
    samples=[]
    for i in range(NB_CHANNELS):
        filepath = Path(filename)/f"ch_{i+1}.bin"
        with open(filepath, "rb") as f:
            # Header minimal : fe (uint32)
            fe, = struct.unpack("<I", f.read(4))
            # print(f"Fréquence d'échantillonnage: {fe} Hz")

            # Lecture de tout le reste du fichier
            # Chaque échantillon = uint16
            samples.append(np.frombuffer(f.read(), dtype=np.uint16))

    # --- Création du vecteur temps ---
    times = np.arange(len(samples[0])) / fe
    
    return times, samples

t,s = extract_data(FILENAME)

# --- Plot ---
fig, axs = plt.subplots(2, 1, figsize=(12, 6))
axs[0].plot(t, s[0], linewidth=1)
axs[0].grid(True)
axs[0].set_ylabel("Valeur ch1 (0-4095)")
axs[0].set_title("Tracé des données acquises")
axs[1].plot(t, s[1], linewidth=1)
axs[1].grid(True)
axs[1].set_xlabel("Temps (s)")
axs[1].set_ylabel("Valeur ch2 (0-4095)")
plt.tight_layout()
plt.show()
