#! /usr/bin/env python2

import atexit
import math
import re
import socket
import sys
import subprocess
import threading
import time

SERVER_SOCKNAME = "/tmp/CS344_manage_sock"
MAX_READ_SIZE = 8192

def terminate_computation():
    print "sending termination signals to all compute instances i have knowledge of and then exiting."

def json_decode(msg):
    mydict = {}
    rx = re.compile(r'"([a-zA-Z]+)":"([a-zA-Z0-9]+)"')
    match = rx.findall(msg)

    for l in match:
        mydict[l[0]] = l[1]

    rx2 = re.compile(r'"([a-zA-Z]+)":({.+})')
    match = rx2.findall(msg)

    for l in match:
        mydict[l[0]] = json_decode(l[1])
        
    return mydict

def handle_report_connection(conn, json_dict):
    conn.send("[packet containing info about processes, their performance, numbers we've tried and the perfect numbers.]")
    if (json_dict["kill"] == "yes"):
        terminate_computation()
    
def handle_compute_connection(conn, json_dict):

    r = int(json_dict['flops'])  # rate
    start = 0   # perfect number lower bound
    end = math.floor(2 + math.sqrt(4 - 2*(-r*15 + (1/2)*start^2 - 2*start)))
    response_json = "{\"job_range\":\"" + str(start) + "-" + str(end) + "\"}"
    print response_json
    conn.send(response_json)
    
    print "handling compute connection: " 

def handle_exit(sock):
    print "I'm exiting and handling it."
    sock.close()
    subprocess.call(["rm", SERVER_SOCKNAME])
    
if __name__ == "__main__":
    # thread to listen for new connections, then update internal data structure that maintains compute processes and their performance
    # once a new communication has started, create a thread that sends that process commands, waits for it to finish, then sends more commands
    print "I'm listening for report connections."

    thread_list = []

    subprocess.call(["rm", "-f", SERVER_SOCKNAME])
    
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.bind(SERVER_SOCKNAME)
    sock.listen(1)

    atexit.register(handle_exit, sock)
    
    while (1):
        conn, addr = sock.accept()
        msg = conn.recv(MAX_READ_SIZE)
        json_dict = json_decode(msg)
        if (json_dict["sender"] == "report"):
            t = threading.Thread(target=handle_report_connection, args=(conn, json_dict,))
            thread_list.append(t)
            t.start()
            
        elif (json_dict["sender"] == "compute"):
            t = threading.Thread(target=handle_compute_connection, args=(conn, json_dict,))
            thread_list.append(t)
            t.start()
            
        else:
            print "Error, unknown message sent to socket. Exiting."
            sys.exit()



