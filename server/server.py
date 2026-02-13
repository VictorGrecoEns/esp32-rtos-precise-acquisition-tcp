import os
from time import time
import logging

from flask import Flask, request, jsonify
import socket

from config import parse_args
from acquisition.controller import AcquisitionController, NB_CHANNELS
from acquisition.storage import DataStorage, BLOC_SIZE


logging.getLogger("werkzeug").setLevel(logging.WARNING)
import numpy as np

FILE_NAME = "vg10022026_acq_1"

args = parse_args()
n_blocks = int(args.duration_s * args.freq_ech / BLOC_SIZE) + 1

app = Flask(__name__)
controller = AcquisitionController(n_blocks)


@app.route("/config", methods=["GET"])
def config():
    return jsonify({
        "n_blocks": n_blocks,
        "fe": args.freq_ech,
    })


@app.route("/init", methods=["POST"])
def init():
    data = request.get_json()
    fe = int(data["fe"])

    storages = []
    for i in range(NB_CHANNELS):
        filePath = os.path.join(
                os.path.dirname(__file__), "data", FILE_NAME,
            )
        os.makedirs(filePath, exist_ok=True)
        storage = DataStorage(
            filepath=filePath+f"/ch_{i+1}.bin",
            fe=fe,
        )
    
        storages.append(storage)

    controller.init(fe, storages)
    controller.init_display(args)

    return "INIT OK", 200


@app.route("/data", methods=["POST"])
def data():
    payload = request.get_json()
    
    samples = [payload[f"samples{i+1}"] for i in range(NB_CHANNELS)]
    controller.delay = time() - controller.delay

    controller.process_block(
        timestamp=int(payload["timestamp"]),
        samples=samples,
    )
    # print(samples[1][:5])
    controller.update_display(samples=samples)

    if controller.is_finished():
        controller.close()
        sizeOfFiles = np.array([os.path.getsize(os.path.dirname(__file__) + f"/data/{FILE_NAME}/ch_{i+1}.bin") for i in range(NB_CHANNELS)]).mean()
        print(f"\nAcquisition terminée en {controller.timeTot:.1f} s| {sizeOfFiles * 1e-3:.2f} ko")
    controller.delay = time()
    return "OK", 200

def get_ip_add():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
    finally:
        s.close()
    return ip

if __name__ == "__main__":
    
    print(f"Running on : {get_ip_add()}:{args.port}")
    app.run(host=args.host, port=args.port)
