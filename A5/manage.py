#! /usr/bin/env python2

import socket

SOCKNAME = "/tmp/CS344_manage_sock"


def listen_for_report_connections():
    # listens for report connections, opens communication socket, gets request, gets information, returns information, sends custom kill signal to main, then kills process
    print "I'm listening for report connections."


def listen_for_compute_connections():
    # thread to listen for new connections, then update internal data structure that maintains compute processes and their performance
    # once a new communication has started, create a thread that sends that process commands, waits for it to finish, then sends more commands
    print "I'm listening for report connections."
    s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    s.bind(SOCKNAME)
    s.listen(1)
    conn, addr = s.accept()




if __name__ == "__main__":
    # main function. create thread to run listen_for_report_connections
    # create thread to run listen_for_report_connections
    listen_for_compute_connections()
