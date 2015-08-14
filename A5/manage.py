#! /usr/bin/env python3

import atexit
import math
import re
import socket
import sys
import subprocess
import threading
import time
import os
import signal

SERVER_SOCKNAME = "/tmp/CS344_manage_sock"
MAX_READ_SIZE = 8192

e = threading.Event()

processes = []
current_num = 1
perf_nums = []

def json_decode(msg):
    mydict = {}
    rx = re.compile(r'"([a-zA-Z]+)":"([a-zA-Z0-9]+)"')
    match = rx.findall(str(msg))

    for l in match:
        mydict[l[0]] = l[1]

    rx2 = re.compile(r'"([a-zA-Z]+)":({.+})')
    match = rx2.findall(str(msg))

    for l in match:
        mydict[l[0]] = json_decode(l[1])
        
    return mydict

def handle_report_connection(conn, json_dict):
    global perf_nums
    
    outstr = "{\"perfects\":\""
    for p in perf_nums:
        outstr = outstr + str(p) + ","

    outstr = outstr + "\",\"search-range\":\"1-" + str(current_num-1) + "\",\"processes\":\""
    for proc in processes:
        outstr = outstr + str(proc["pid"]) + ","

    outstr = outstr + "\"}"
        
    conn.send(str.encode(outstr))

    if (json_dict["kill"] == "yes"):
        handle_exit()
    
def handle_compute_connection(conn, json_dict):

    global current_num
    global processes
    global perf_nums
    
    proc_dict = {}
    proc_dict["pid"] = json_dict["pid"]
    proc_dict["flops"] = json_dict["flops"]
    
    r = int(json_dict['flops'])  # rate
    start = current_num   # perfect number lower bound
    end = int(math.sqrt(2*r*15+start*start))
    current_num = end + 1
    
    proc_dict["start"] = start
    proc_dict["end"] = end
    processes.append(proc_dict)
    
    response_json = str.encode("{\"job_range\":\"" + str(start) + "-" + str(end) + "\"}")
    
    conn.send(response_json)
    json_data = conn.recv( (end - start) + 13)
    parsed_data = json_decode(json_data)

    try:
        for i in range(0, len(parsed_data["data"])):
            if (parsed_data["data"][i] == '1'):
                perf_nums.append(i+start)

    except:
        pass
    processes.remove(proc_dict)

def handle_term_connection(conn, json_dict):
    global e
    e.wait()
    try:
        conn.send(b'{"kill":""}')
    except:
        pass
           
def handle_exit():
    global e
    e.set()
    e.clear()
    time.sleep(1)
    subprocess.call(["rm", SERVER_SOCKNAME])
    print("manage.py exiting.")
    os._exit(0)

def sig_handle_exit(one, two):
    global e
    e.set()
    e.clear()
    time.sleep(1)
    subprocess.call(["rm", SERVER_SOCKNAME])
    print("manage.py exiting.")
    os._exit(0)


def handle_new_connection(conn):
    msg = conn.recv(MAX_READ_SIZE)
    json_dict = json_decode(msg)
    if ("sender" not in json_dict.keys()):
        sys.exit()
        
    if (json_dict["sender"] == "report"):
        t = threading.Thread(target=handle_report_connection, args=(conn, json_dict,))
        t.start()
            
    elif (json_dict["sender"] == "compute"):
        t = threading.Thread(target=handle_compute_connection, args=(conn, json_dict,))
        t.start()

    elif (json_dict["sender"] == "term"):
        t = threading.Thread(target=handle_term_connection, args=(conn, json_dict,))
        t.start()
            
    else:
        print( "Error, unknown message sent to socket. Exiting.")
        sys.exit()
    sys.exit()

    
if __name__ == "__main__":
    # thread to listen for new connections, then update internal data structure that maintains compute processes and their performance
    # once a new communication has started, create a thread that sends that process commands, waits for it to finish, then sends more commands

    signal.signal(signal.SIGINT, sig_handle_exit)
    signal.signal(signal.SIGQUIT, sig_handle_exit)
    subprocess.call(["rm", "-f", SERVER_SOCKNAME])
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.bind(SERVER_SOCKNAME)
    sock.listen(1)

    atexit.register(handle_exit)
    
    while (1):
        conn, addr = sock.accept()
        t = threading.Thread(target=handle_new_connection, args=(conn,))
        t.start()

