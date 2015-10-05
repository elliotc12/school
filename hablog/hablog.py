#! /usr/bin/env python
import sys
import time

def read_loop(read_inc):
	print "Ready, set, read!"
	time.sleep(60*read_inc)
	print "\a\a\a\a\a\a"
	pages = float(raw_input("How many pages did you read? "))
	comments = raw_input("Any special comments? ")
	return (pages, comments)

def read():
	read_inc = int(raw_input("How long should the reading increment be in minutes? "))
	fd = open("log.txt", 'a')
	read_mins = 0
	rerun = 'y'
	
	raw_input("Hit enter to start!")
	
	while (rerun == 'y'):
		(pages, comments) = read_loop(read_inc)
		ts = time.time()
		fd.write("{}\t{}\t{}\t{}\n".format(*[ts, read_inc, pages, comments]))
		read_mins += read_inc
		rerun = raw_input("Again (y/n)? ")
	
	print "Read a total of {} minutes!".format(*[read_mins])


def rate():
	print "rating"

def main(argv):
	if (len(argv) > 1):
		print "Too many arguments. Correct usage: ./hablog.py [read/rate]"
		exit(1)
		
	if argv[0] == "read":
		read()
		exit(0)
		
	elif argv[0] == "rate":
		rate()
		exit(0)
		
	else:
		print "Unrecognized argument. Correct usage: ./hablog.py [read/rate]"
		exit(1)

if __name__ == "__main__":
	main(sys.argv[1:])