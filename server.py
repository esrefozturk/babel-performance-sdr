import socket

UDP_IP = "10.10.10.100"
UDP_PORT = 8000

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))


while True:
    data, addr = sock.recvfrom(128) # buffer size is 128 bytes
    print "received message:", data