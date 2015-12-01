#! /usr/bin/env python

from __future__ import division

import math
import numpy as np
import sys
import time
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab

plt.ion()

data = np.genfromtxt("data.txt")

if len(sys.argv[1:]) == 0:
    hi = raw_input("Press any key to play animation.")

time = 10 # s
frames = len(data) / 10

ax = plt.gca()
ax.set_aspect("equal", adjustable="box")
#ax.set_xlim(-40,40)
#ax.set_ylim(-40,40)

i = 0

datafile = open("data.txt", "r").read()

if  len(data.shape) == 1:
    Z = np.reshape(data, (math.sqrt(len(data)), math.sqrt(len(data))))
    plt.imshow(Z)
    input("Press enter to continue.")
    sys.exit(0)
    
else:
    while i < len(data):
        print str(i) + " of " + str(len(data))
        Z = np.reshape(data[i], (math.sqrt(len(data[i])), math.sqrt(len(data[i]))))
        plt.imshow(Z)
        plt.pause(0.001)
        i += math.ceil(len(data)/frames)
