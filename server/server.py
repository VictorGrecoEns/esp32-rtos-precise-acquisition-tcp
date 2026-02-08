import os
from time import time
import logging

from flask import Flask, request, jsonify

from config import parse_args
from acquisition.controller import AcquisitionController
from acquisition.storage import DataStorage, BLOC_SIZE


logging.getLogger("werkzeug").setLevel(logging.WARNING)

FILE_NAME = "vg06022026_acq_1"

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

    storage = DataStorage(
        filepath=os.path.join(
            os.path.dirname(__file__),
            "data",
            f"{FILE_NAME}.bin",
        ),
        fe=fe,
    )

    controller.init(fe, storage)

    controller.console.print()
    controller.console.print(f"[bold]Port[/bold]                      = {args.host}:{args.port}")
    controller.console.print(f"[bold]Durée acquisition[/bold]         = {args.duration_s} s")
    controller.console.print(f"[bold]Fréquence échantillonnage[/bold] = {controller.fe} Hz")
    controller.console.print(f"[bold]Nombre de blocs[/bold]           = {controller.n_blocks}")
    controller.console.print()

    return "INIT OK", 200


@app.route("/data", methods=["POST"])
def data():
    payload = request.get_json()

    controller.delay = time() - controller.delay

    controller.process_block(
        timestamp=int(payload["timestamp"]),
        samples=payload["samples"],
    )

    if controller.is_finished():
        controller.close()

        filepath = os.path.join(
            os.path.dirname(__file__),
            "data",
            f"{FILE_NAME}.bin",
        )
        size_kb = os.path.getsize(filepath) * 1e-3

        controller.console.print(
            f"\n[bold green]Acquisition terminée[/bold green] "
            f"({size_kb:.2f} ko)"
        )

    controller.delay = time()
    return "OK", 200

if __name__ == "__main__":
    app.run(host=args.host, port=args.port)
