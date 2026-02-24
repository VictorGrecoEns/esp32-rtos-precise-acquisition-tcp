import socket

class TCPServer:

    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.sock = None
        self.conn = socket.socket()

    def start(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.bind((self.host, self.port))
        self.sock.listen(1)
        print(f"Serveur en écoute sur {self.host}:{self.port}")

        self.conn, addr = self.sock.accept()
        print("Connecté par", addr)

    def send(self, data: bytes):
        self.conn.sendall(data)

    def recv_exact(self, size: int) -> bytes:
        data = b''
        while len(data) < size:
            chunk = self.conn.recv(size - len(data))
            if not chunk:
                raise ConnectionError("Connexion interrompue")
            data += chunk
        return data

    def close(self):
        if self.conn:
            self.conn.close()
        if self.sock:
            self.sock.close()