from flask import Flask, request, jsonify
import os
from save_data import DataStorage

# Paramètres de l'acquisition
nbBlocToSend    = 10
nbBlocReceived  = 0
blockSize       = 0

# Buffer pour stocker temporairement les blocs reçus dans l'ordre
blocks_buffer = []

last_timestamp  = None
storage:DataStorage 

app = Flask(__name__)

@app.route("/config", methods=["GET"])
def config():
    return jsonify({
        "n_blocks": nbBlocToSend
    })

@app.route("/init", methods=["POST"])
def init_acquisition():
    global storage, nbBlocReceived, blockSize, last_timestamp, blocks_buffer

    data      = request.get_json()
    fe        = int(data["fe"])
    blockSize = int(data["block_size"])
    n_blocks  = int(data["n_blocks"])

    print("Initialisation acquisition :")
    print(f"fe={fe} Hz, block_size = {blockSize}, n_blocks={n_blocks}")

    # Crée le stockage des données
    storage = DataStorage(
        os.path.join(os.path.dirname(__file__), "data/vg06022026_acq_1.bin"),
        block_size=blockSize,
        fe=fe
    )

    nbBlocReceived = 0
    last_timestamp = None
    blocks_buffer  = []

    return "INIT OK", 200

@app.route("/data", methods=["POST"])
def receive_data():
    global last_timestamp, nbBlocReceived, blockSize, storage, blocks_buffer

    data          = request.get_json()
    timestamp_us  = int(data["timestamp"])
    samples       = data["samples"]
    bloc_num      = data.get("bloc", nbBlocReceived)

    # Ajoute le bloc au buffer FIFO
    blocks_buffer.append((timestamp_us, bloc_num, samples))
    nbBlocReceived += 1

    # Traite le bloc immédiatement pour respecter l'ordre d'acquisition
    timestamp, bloc, samples = blocks_buffer.pop(0)

    if last_timestamp is not None:
        if timestamp <= last_timestamp:
            print(f"Bloc reçu hors ordre ! timestamp={timestamp} <= last_timestamp={last_timestamp}")
        else:
            dt = timestamp - last_timestamp
            fe_eval = blockSize * 1e6 / dt
            print(rf"Bloc {bloc} / {nbBlocReceived} OK, timestamp_us={timestamp} $\Delta t$ = {dt*1e-3:.2f} ms par bloc (fe_eval = {fe_eval:.2f} Hz)")
    else:
        print(f"Bloc {bloc} / {nbBlocReceived} OK, timestamp_us={timestamp}")

    last_timestamp = timestamp

    # Stocke les échantillons
    storage.append_block(samples)

    return "OK", 200

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
