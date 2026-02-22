import socket
import struct
import os

# ====================
# Paramètres serveur
# ====================
HOST = '192.168.17.7'
PORT = 5000
NB_CAPTEURS = 2
SAMPLES_PER_CAPTEUR = 256
FRAME_SIZE = SAMPLES_PER_CAPTEUR * NB_CAPTEURS
N_BUFFERS = 4  # nombre exact de buffers à recevoir
DATA_NAME = "vg22022026"

# Créer un dossier pour stocker les fichiers si inexistant
directory = os.path.dirname(__file__)
DIR_PATH = directory + "/data/" + DATA_NAME
os.makedirs(DIR_PATH, exist_ok=True)
files = [open(DIR_PATH + f"/ch_{i+1}.bin", "wb") for i in range(NB_CAPTEURS)]

# ====================
# Socket TCP
# ====================
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    s.bind((HOST, PORT))
    s.listen(1)
    print(f"Serveur en écoute sur {HOST}:{PORT}...")
except Exception as e:
    print(f"Erreur de connexion serveur : {e}")
    exit(1)


try:
    conn, addr = s.accept()
    print("Connecté par", addr)

    # ====================
    # Demander le start à l'ESP32
    # ====================
    cmd = struct.pack('<BH', 1, N_BUFFERS)  # 1 = START, H = uint16_t buffers
    conn.sendall(cmd)

    buffers_received = 0
    
    while buffers_received < N_BUFFERS:
        # Lire le timestamp (8 octets uint64_t)
        data = conn.recv(8)
        if len(data) < 8:
            break
        timestamp, = struct.unpack('<Q', data)

        # Lire le buffer complet
        buffer_bytes = FRAME_SIZE * 2
        buf_data = b''
        while len(buf_data) < buffer_bytes:
            chunk = conn.recv(buffer_bytes - len(buf_data))
            if not chunk:
                break
            buf_data += chunk

        # Convertir en liste d'entiers 16 bits
        samples = list(struct.unpack('<' + 'H'*FRAME_SIZE, buf_data))

        # Écrire les échantillons par capteur
        for cap in range(NB_CAPTEURS):
            cap_samples = samples[cap::NB_CAPTEURS]
            files[cap].write(struct.pack('<' + 'H'*len(cap_samples), *cap_samples))

        print(f"{buffers_received} - {timestamp=}")
        buffers_received += 1
        
    # Stop ESP32
    conn.sendall(b'STOP\n')
    print(f"{buffers_received} buffers reçus, acquisition terminée.")

finally:
    for f in files:
        f.close()
    conn.close()
    s.close()