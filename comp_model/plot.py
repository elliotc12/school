#! /usr/bin/env python

import math
import numpy as np
import time
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab

data = np.genfromtxt("data.txt")
print len(data)
plt.ion()

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
    plt.pause(1)
    i+=1

plt.show()
