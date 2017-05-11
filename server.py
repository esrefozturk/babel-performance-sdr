import socket
from time import time
from sys import argv

UDP_IP = "10.10.10.100"
UDP_PORT = 8000

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))

filename = argv[1]

f = open(filename, 'w')

while True:
    sending_time, addr = sock.recvfrom(128) # buffer size is 128 bytes
    if sending_time == 'STOP':
        break
    getting_time = repr(time())
    f.write(sending_time+','+getting_time+'\n')
f.close()