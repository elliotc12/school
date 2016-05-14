#! /usr/bin/python2.7

from __future__ import division

import numpy as np
import matplotlib.pyplot as plt

I = []

data = np.genfromtxt('seq.txt', dtype='string')
K = data[0]
seq = data[1:]
n = len(seq)
l = len(seq[0])
pa = 0
pt = 0
pc = 0
pg = 0
A = 0
T = 1
G = 2
C = 3

for s in seq:
    for i in range(len(s)):
        pa += (s[i] == 'A') * 1 / (n*l)
        pt += (s[i] == 'T') * 1 / (n*l)
        pg += (s[i] == 'G') * 1 / (n*l)
        pc += (s[i] == 'C') * 1 / (n*l)

W = np.zeros([4, l])
for s in seq:
    for i in range(len(s)):
        W[A][i] += (s[i] == 'A') * 1 / (n * pa) # initialize W via definition
        W[T][i] += (s[i] == 'T') * 1 / (n * pt)
        W[G][i] += (s[i] == 'G') * 1 / (n * pg)
        W[C][i] += (s[i] == 'C') * 1 / (n * pc)

print W
W = np.log(W)
print W

def S(x, W):
    score = 0
    for i in range(len(x)):
        score += (x[i] == 'A') * W[A][i]
        score += (x[i] == 'T') * W[T][i]
        score += (x[i] == 'G') * W[G][i]
        score += (x[i] == 'C') * W[C][i]

# while (S(x) - s) / S(x) > 0.1:
#     dostuff
