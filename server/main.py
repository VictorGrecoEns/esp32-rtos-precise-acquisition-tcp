from config import parse_args, compute_n_buffers, make_repo
from network.tcp_server import TCPServer
from acquisition.session import AcquisitionSession

def main():
    args = parse_args()
    
    n_buffers = compute_n_buffers(args.duration_s, args.freq)

    server = TCPServer(args.host, args.port)
    
    session = AcquisitionSession(server, n_buffers, args.freq, make_repo(args.repo, createRepo=True))

    session.run()

if __name__ == "__main__":
    main()