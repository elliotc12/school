#! /usr/bin/env python2

import atexit
import socket
import threading
import sys
import time
import subprocess

SERVER_SOCKNAME = "/tmp/CS344_manage_sock"
MAX_READ_SIZE = 8192

def json_decode(msg):
    print "I'm going to decode: " + msg

    
    
    return "compute"

def handle_report_connection(conn, msg):
    print "handling report connection" + msg
    time.sleep(1000)
    
def handle_compute_connection(conn, msg):
    print "handling compute connection: " + msg
    
    time.sleep(1000)

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
        sender = json_decode(msg)
        if (sender == "report"):
            t = threading.Thread(target=handle_report_connection, args=(conn, msg,))
            thread_list.append(t)
            t.start()
            
        elif (sender == "compute"):
            t = threading.Thread(target=handle_compute_connection, args=(conn, msg,))
            thread_list.append(t)
            t.start()
            
        else:
            print "Error, unknown message sent to socket. Exiting."
            sys.exit()



