#! /usr/bin/env python2
import socket
import getopt
import sys

SERVER_SOCKNAME = "/tmp/CS344_manage_sock"
MAX_READ_SIZE = 8192

if __name__ == "__main__":

    killmode = False
    
    if (sys.argv[1:] == ['-k']):
        killmode = True
    
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.connect(SERVER_SOCKNAME)

    msg = '{"sender":"report",'
    if (killmode):
        msg = msg + '"kill":"yes"}'
    else:
        msg = msg + '"kill":"no"}'
        
    sock.send(msg)
    report = sock.recv(MAX_READ_SIZE)
    sock.close()

    print report