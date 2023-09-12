from pythonosc.osc_server import BlockingOSCUDPServer
from pythonosc.dispatcher import Dispatcher

IP_MULTICAST = "224.0.0.1"
PORT = 7401

def handler(address, *args):
    print(f"{address}: {args}")
    print("Starting SLMX4 transmission to Max")
    

def default_handler(address, *args):
    print(f"DEFAULT {address}: {args}")
    
dispatcher = Dispatcher()
dispatcher.map("/address", handler)
dispatcher.set_default_handler(default_handler)

server = BlockingOSCUDPServer((IP_MULTICAST, PORT), dispatcher)
print("Start server")
server.serve_forever()

