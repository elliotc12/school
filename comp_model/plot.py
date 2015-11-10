#! /usr/bin/env python

from __future__ import division

import math
import numpy as np
import time
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab

plt.ion()

data = np.genfromtxt("data.txt")
time = 1 # s
frames = 10

ax = plt.gca()
ax.set_aspect("equal", adjustable="box")
#ax.set_xlim(-40,40)
#ax.set_ylim(-40,40)

plt.xlabel('$x$ (m)')
plt.ylabel('$y$ (m)')

i = 0
while i < len(data):
    print str(i) + " of " + str(len(data))
    Z = np.reshape(data[i], (math.sqrt(len(data[i])), math.sqrt(len(data[i]))))
    plt.imshow(Z)
    plt.pause(time/frames)
    i += int(len(data)/frames)

plt.show()
