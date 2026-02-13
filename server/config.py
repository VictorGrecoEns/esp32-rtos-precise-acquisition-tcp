import argparse

def parse_args():
    parser = argparse.ArgumentParser(
        description="Serveur Flask d'acquisition"
    )

    parser.add_argument(
        "--freq-ech",
        type=int,
        default=100,
        help="Nombre de blocs à recevoir"
    )
    
    parser.add_argument(
        "--duration-s",
        type=int,
        default=5,
        help="Durée de l'acquisition"
    )

    parser.add_argument(
        "--host",
        default="0.0.0.0"
    )

    parser.add_argument(
        "--port",
        type=int,
        default=5000
    )

    return parser.parse_args()
