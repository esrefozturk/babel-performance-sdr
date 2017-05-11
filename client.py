import socket
from time import sleep

UDP_IP = "10.10.10.100"
UDP_PORT = 8000
MESSAGE = "ESREF"

print "UDP target IP:", UDP_IP
print "UDP target port:", UDP_PORT
print "message:", MESSAGE

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

for i in range(1,1001):
    sock.sendto(str(i), (UDP_IP, UDP_PORT))
    sleep(0.05)