#! /usr/bin/python2.7

from __future__ import division

import numpy as np
import matplotlib.pyplot as plt
import random

random.seed()

N = 100

data = np.genfromtxt('seq.txt', dtype='string')
K = int(data[0])
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

def get_W(sequence):
    f = np.zeros([4, K])
    for s in sequence:
        for i in range(K):
            f[A][i] += (s[i] == 'A')
            f[T][i] += (s[i] == 'T')
            f[G][i] += (s[i] == 'G')
            f[C][i] += (s[i] == 'C')
    f /= n
    for i in range(len(f[A])):
        if f[A][i] == 0:
            f[A][i] = -1e-100
        else:
            f[A][i] = np.log(f[A][i]/pa)
    for i in range(len(f[T])):
        if f[T][i] == 0:
            f[T][i] = -1e-100
        else:
            f[T][i] = np.log(f[T][i]/pt)
    for i in range(len(f[G])):
        if f[G][i] == 0:
            f[G][i] = -1e-100
        else:
            f[G] = np.log(f[G][i]/pg)
    for i in range(len(f[C])):
        if f[C][i] == 0:
            f[C][i] = -1e-100
        else:
            f[C][i] = np.log(f[C][i]/pc)
    return f

def get_HM(sequence):
    f = np.zeros([4, K])
    H_M = np.zeros([K])
    for s in sequence:
        for i in range(K):
            f[A][i] += (s[i] == 'A') # initialize W via definition
            f[T][i] += (s[i] == 'T')
            f[G][i] += (s[i] == 'G')
            f[C][i] += (s[i] == 'C')
    for i in range(K):
        if f[A][i] != 0:
            H_M[i] += -f[A][i]*np.log(f[A][i])
        if f[T][i] != 0:
            H_M[i] += -f[T][i]*np.log(f[T][i])
        if f[G][i] != 0:
            H_M[i] += -f[G][i]*np.log(f[G][i])
        if f[C][i] != 0:
            H_M[i] += -f[C][i]*np.log(f[C][i])
    return H_M

def S(s, W):
    score = 0
    for i in range(len(s)):
        score += (s[i] == 'A') * W[A][i]
        score += (s[i] == 'T') * W[T][i]
        score += (s[i] == 'G') * W[G][i]
        score += (s[i] == 'C') * W[C][i]
    return score

for s in seq:
    for i in range(len(s)):
        pa += (s[i] == 'A') * 1 / (n*l)
        pt += (s[i] == 'T') * 1 / (n*l)
        pg += (s[i] == 'G') * 1 / (n*l)
        pc += (s[i] == 'C') * 1 / (n*l)

H_bg = -pa*np.log(pa) - pt*np.log(pt) - pg*np.log(pg) - pc*np.log(pc)

j = np.empty([n]).astype(int)
for i in range(n):
    j[i] = random.randint(0,l-K)

info = []

for x in range(N):
    guess_motif = []
    for i in range(n):     # create new weight matrix
        guess_motif.append(seq[i][j[i]:j[i]+K])
    W = get_W(guess_motif)
    H_M = get_HM(guess_motif)

    S0 = np.empty([n])     # get new scores
    for i in range(n):
        S0[i] = S(guess_motif[i], W)

    for i, s in enumerate(seq):   # update guess motif by weight matrix
        for k in range(l-K):
            if S(s[k], W) < S0[i]:
                S0[i] = S(s[i], W)
                j[i] = k

    I = H_bg - sum(H_M)/K
    print guess_motif
    print I
    info.append(I)
    #print np.sum(S0)

plt.plot(info)
plt.xlabel("Iteration")
plt.ylabel("Information")
plt.title("Information content per iteration in Gibb's Sampling")
plt.show()
