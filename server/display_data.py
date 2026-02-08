import struct
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
import os
# Chemin du fichier binaire
FILENAME = os.path.dirname(__file__) + "/data/vg06022026_acq_1.bin"

def extract_data(filename):
    # --- Lecture du fichier ---
    filepath = Path(filename)
    with open(filepath, "rb") as f:
        # Header minimal : fe (uint32)
        fe, = struct.unpack("<I", f.read(4))
        # print(f"Fréquence d'échantillonnage: {fe} Hz")

        # Lecture de tout le reste du fichier
        # Chaque échantillon = uint16
        samples = np.frombuffer(f.read(), dtype=np.uint16)

    # --- Création du vecteur temps ---
    times = np.arange(len(samples)) / fe
    
    return times, samples

t,s = extract_data(FILENAME)

# --- Plot ---
plt.figure(figsize=(12, 4))
plt.plot(t, s, linewidth=1)
plt.xlabel("Temps (s)")
plt.ylabel("Valeur (0-4095)")
plt.title("Tracé des données acquises")
plt.grid(True)
plt.tight_layout()
plt.show()
