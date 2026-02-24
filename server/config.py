import argparse
from os.path import join, dirname
from os import makedirs
from datetime import datetime

HOST = '192.168.17.7'
PORT = 5000
NB_CAPTEURS = 2
SAMPLES_PER_CAPTEUR = 256
SAMPLE_FREQUENCY = 100
FRAME_SIZE = SAMPLES_PER_CAPTEUR * NB_CAPTEURS
DATA_NAME = f"vg{datetime.now():%d%m%Y}"


def parse_args():
    parser = argparse.ArgumentParser(
        description="Serveur Flask d'acquisition"
    )

    parser.add_argument(
        "--duration-s",
        type=int,
        default=5,
        help="Durée de l'acquisition en seconde"
    )
    
    parser.add_argument(
        "--freq",
        type=int,
        default=SAMPLE_FREQUENCY,
        help="Fréquence de l'acquisition en Hz"
    )

    parser.add_argument(
        "--host",
        default=HOST,
        help="Adresse IP du serveur HOST, format: 'XXX:XXX:XXX:XXX'"
    )

    parser.add_argument(
        "--port",
        type=int,
        default=PORT,
        help="Port ouvert à la communication"
    )

    parser.add_argument(
        "--repo",
        type=str,
        default=DATA_NAME,
        help="Nom du répertoire de l'acquisition dans ./data/"
    )
    
    return parser.parse_args()

def compute_n_buffers(duration_s, freq_acq):
    n = int(duration_s * freq_acq / SAMPLES_PER_CAPTEUR) + 1
    print(f"Acquisition de {n} buffers à {freq_acq}Hz")
    print(f"Estimation temps total: {n*SAMPLES_PER_CAPTEUR/freq_acq:.2f} s | {1e3*SAMPLES_PER_CAPTEUR/freq_acq:.2f}ms/buffer")
    return n

def make_repo(repository):
    repo = join(dirname(__file__), "data", repository)
    makedirs(repo, exist_ok=True)
    return repo