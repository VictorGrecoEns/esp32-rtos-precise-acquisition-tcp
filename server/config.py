import argparse
import os 

HOST = '192.168.17.7'
PORT = 5000
NB_CAPTEURS = 2
SAMPLES_PER_CAPTEUR = 256
SAMPLE_FREQUENCY = 100
FRAME_SIZE = SAMPLES_PER_CAPTEUR * NB_CAPTEURS
DATA_NAME = "/data/vg22022026"

# Créer un dossier pour stocker les fichiers si inexistant
DIR_PATH = os.path.dirname(__file__) + DATA_NAME
os.makedirs(DIR_PATH, exist_ok=True)


def parse_args():
    parser = argparse.ArgumentParser(
        description="Serveur Flask d'acquisition"
    )

    parser.add_argument(
        "--duration-s",
        type=int,
        default=5,
        help="Durée de l'acquisition"
    )
    
    parser.add_argument(
        "--freq",
        type=int,
        default=SAMPLE_FREQUENCY,
        help="Fréquence de l'acquisition"
    )

    parser.add_argument(
        "--host",
        default=HOST,
        help="Adresse IP du serveur HOST"
    )

    parser.add_argument(
        "--port",
        type=int,
        default=PORT,
        help="Port ouvert à la communication"
    )

    return parser.parse_args()

def compute_n_buffers(duration_s, freq_acq):
    n = int(duration_s * freq_acq / SAMPLES_PER_CAPTEUR) + 1
    print(f"Acquisition de {n} buffers")
    print(f"Estimation temps total: {n*SAMPLES_PER_CAPTEUR/freq_acq:.2f} s | {1e3*SAMPLES_PER_CAPTEUR/freq_acq:.2f}ms/buffer")
    return n