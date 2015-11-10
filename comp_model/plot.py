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
time = 1 # s
frames = 50

ax = plt.gca()
ax.set_aspect("equal", adjustable="box")
#ax.set_xlim(-40,40)
#ax.set_ylim(-40,40)

plt.xlabel('$x$ (m)')
plt.ylabel('$y$ (m)')
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
        plt.pause(time/frames)
        i += int(len(data)/frames)
