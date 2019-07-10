#!/bin/bash
import socket
import os, os.path

'Socket server address'
server_address = "/tmp/logger.sock"

print "Connecting..."
if os.path.exists( server_address):
  client = socket.socket( socket.AF_UNIX, socket.SOCK_DGRAM )
  client.connect( server_address )
  client.send("Data to log")
  client.close()
else:
  print "Couldn't Connect!"
 
