import socket
from time import sleep

UDP_IP = "20.20.20.1"
UDP_PORT = 8000
MESSAGE = "Hello, World!"

print "UDP target IP:", UDP_IP
print "UDP target port:", UDP_PORT
print "message:", MESSAGE

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

while True:
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
    sleep(0.05)