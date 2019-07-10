import socket
import os, os.path
import time

'Socket server address'
server_address = "/tmp/logger.sock"

'Make sure file doesnt exists already'
if os.path.exists(server_address):
  os.remove(server_address)

print "Opening Socket"

'Create a UDP UDS socket'
server = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)

'Bind the socket to the port'
print('Starting up on {}'.format(server_address))
server.bind(server_address)

print "Listening..."

while True:
  data = server.recv(1024)
  if not data:
    break
  else:
    print "data : " + data

server.close()
os.remove(server_address)
print "finish"

