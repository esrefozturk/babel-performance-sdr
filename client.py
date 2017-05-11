import socket
from time import sleep, time
from sys import argv

UDP_IP = "10.10.10.100"
UDP_PORT = 8000

N = 1000

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

filename = argv[1]

f = open(filename, 'w')

for i in range(N):
    now = repr(time())
    sock.sendto(now, (UDP_IP, UDP_PORT))
    f.write(now+'\n')
    sleep(0.05)
for i in range(100):
    sock.sendto("STOP", (UDP_IP, UDP_PORT))
f.close()