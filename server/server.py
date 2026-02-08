import os
from time import time
import sys

import logging
logging.getLogger('werkzeug').setLevel(logging.WARNING)

from flask import Flask, request, jsonify

from config import parse_args
from acquisition.controller import AcquisitionController, middleLine, emptyLine, linesToDisplay
from acquisition.storage import DataStorage, BLOC_SIZE
from plotting.liveplot import LivePlot

args = parse_args()
n_blocks = int(args.duration_s * args.freq_ech / BLOC_SIZE) + 1

app = Flask(__name__)
controller = AcquisitionController(n_blocks)

@app.route("/config", methods=["GET"])
def config():
    return jsonify({"n_blocks": n_blocks, "fe":args.freq_ech})

@app.route("/init", methods=["POST"])
def init():
    data = request.get_json()
    fe = int(data["fe"])

    storage = DataStorage(
        filepath=os.path.dirname(__file__) + "/data/vg06022026_acq_1.bin",
        fe=fe
    )

    controller.init(fe, storage)
    print(f"\nDuree de l'acquisition      = {args.duration_s}s")
    print(f"Fréquence d'échantillonnage = {controller.fe } Hz")
    print(f"Nombre de bloc attendus     = {controller.n_blocks}\n\n")
    print(middleLine)
    print(f"|  Reçu | Total | Moyenne éch | Délai récep (ms) |")
    print(middleLine)
    print((linesToDisplay) * f"{emptyLine}\n")
    return "INIT OK", 200

@app.route("/data", methods=["POST"])
def data():
    payload = request.get_json()
    controller.delay = time() - controller.delay
    controller.process_block(
        timestamp=int(payload["timestamp"]),
        samples=payload["samples"]
    )

    if controller.is_finished():
        controller.close()
        print(middleLine)
        sizeOfFile = os.path.getsize(os.path.dirname(__file__) + "/data/vg06022026_acq_1.bin")
        print(f"\nAcquisition terminée ({sizeOfFile * 1e-3:.2f} ko)")
    controller.delay = time()
    return "OK", 200

if __name__ == "__main__":
    
    app.run(host=args.host, port=args.port)
