#! /usr/bin/env python3
import socket
import getopt
import sys
import re
import binascii

SERVER_SOCKNAME = "/tmp/CS344_manage_sock"
MAX_READ_SIZE = 8192

def json_decode(msg):
    mydict = {}
    rx = re.compile(r'"([a-zA-Z\-]+)":"([a-zA-Z0-9,\-]*)"')
    match = rx.findall(msg)

    for l in match:
        mydict[l[0]] = l[1]
        
    return mydict

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
        
    sock.send(str.encode(msg))
    report = sock.recv(MAX_READ_SIZE)
    report = json_decode(report.decode("utf-8"))
    
    sock.close()

    if (str(report["processes"]) == ""):
        print("processes running: None")
    else:
        print("processes running: " + str(report["processes"])[:-1])
    print("perfect numbers found: " + str(report["perfects"])[:-1])
    if (str(report["search-range"]) == "1-0"):
        print ("Search range: Nothing searched yet")
    else:
        print("numbers searched: " + str(report["search-range"]))
