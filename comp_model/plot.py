#! /usr/bin/env python

import math
import numpy as np
import time
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab

data = np.genfromtxt("data.txt", delimiter=" ")
plt.ion()

ax = plt.gca()
ax.set_aspect("equal", adjustable="box")
#ax.set_xlim(-40,40)
#ax.set_ylim(-40,40)

plt.xlabel('$x$ (m)')
plt.ylabel('$y$ (m)')

i = 0

x = range(0, int(math.sqrt(len(data))))
y = range(0, int(math.sqrt(len(data))))
X, Y = np.meshgrid(x, y)
#Z = np.reshape(data, (math.sqrt(len(data)), math.sqrt(len(data))))

Z1 = mlab.bivariate_normal(X, Y, 1.0, 1.0, 0.0, 0.0)
Z2 = mlab.bivariate_normal(X, Y, 1.5, 0.5, 1, 1)
Z = 10.0 * (Z2 - Z1)

print X, Z

plt.contour(X, Y, Z)

# while i < len(data) or loop:
  
#   if len(sys.argv) >= 2:
#     if sys.argv[1] == "step":
#       raw_input("Hit enter to step.")
#       i += 1
#     else
#       i += 1
#       plt.pause(0.001)

#   plt.draw()
