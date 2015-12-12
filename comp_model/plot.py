#! /usr/bin/env python

from __future__ import division

import math
import numpy as np
import os
import sys
import time
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab

plt.ion()

data = np.genfromtxt("data.txt")

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
    Z = np.reshape(data[0], (math.sqrt(len(data[0])), math.sqrt(len(data[0]))))
    cax = plt.imshow(Z)

    cbar = plt.colorbar(cax)#, ticks=[-1, 0, 1])
    #cbar.ax.set_yticklabels(['< -1', '0', '> 1'])  # vertically oriented colorbar
        
    while i < len(data):
        print str(i) + " of " + str(len(data))
        Z = np.reshape(data[i], (math.sqrt(len(data[i])), math.sqrt(len(data[i]))))
        plt.imshow(Z)
        
        if len(sys.argv[1:]) == 1:        
            fname = 'PNGs/%s-%03d.png' % (sys.argv[1], i)
            plt.savefig(fname)
            
        plt.pause(0.0001)
        i += 1
    
if len(sys.argv[1:]) == 1:        
    os.system("convert -delay 50 PNGs/%s-*.png GIFs/%s.gif" % (sys.argv[1], sys.argv[1]))
