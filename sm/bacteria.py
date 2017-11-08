#!/usr/bin/env python

import numpy as np
import matplotlib
import matplotlib.pyplot as plt

simulations = 10000

length10dist = np.zeros(1001)
length = 10
for _ in range(simulations):
    R = 500
    B = 500
    for _ in range(length):
        R = R*2
        B = B*2
        r = np.random.hypergeometric(R, B, 1000)
        R = R - r
        B = B - (1000-r)
        length10dist[R] += 1
length10dist = length10dist / np.sum(length10dist)
        
length100dist = np.zeros(1001)
length = 100
for _ in range(simulations):
    R = 500
    B = 500
    for _ in range(length):
        R = R*2
        B = B*2
        r = np.random.hypergeometric(R, B, 1000)
        R = R - r
        B = B - (1000-r)
        length100dist[R] += 1
length100dist = length100dist / np.sum(length100dist)

length1000dist = np.zeros(1001)
length = 1000
for _ in range(simulations):
    R = 500
    B = 500
    for _ in range(length):
        R = R*2
        B = B*2
        r = np.random.hypergeometric(R, B, 1000)
        R = R - r
        B = B - (1000-r)
        length1000dist[R] += 1
length1000dist = length1000dist / np.sum(length1000dist)

# length10000dist = np.zeros(1001)
# length = 10000
# for _ in range(simulations):
#     R = 500
#     B = 500
#     for _ in range(length):
#         R = R*2
#         B = B*2
#         r = np.random.hypergeometric(R, B, 1000)
#         R = R - r
#         B = B - (1000-r)
#         length10000dist[R] += 1
# length10000dist = length10000dist / np.sum(length10000dist)

plt.figure()
plt.plot(list(range(1001)), length10dist, label="10h")
plt.plot(list(range(1001)), length100dist, label="100h")
plt.plot(list(range(1001)), length1000dist, label="1000h")
# plt.plot(list(range(1001)), length10000dist, label="10000h")
plt.legend()
plt.title("Red bacteria probability distribution after n hours")
plt.savefig("distribution.png")
