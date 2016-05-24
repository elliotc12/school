#! /usr/bin/python2.7

from __future__ import division

import numpy as np
import matplotlib.pyplot as plt
import random

random.seed()

num_iterations = 20
num_attempts = 1

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

infos  = []
scores = []
datas = []

for a in range(num_attempts):
    j = np.empty([n]).astype(int)
    for i in range(n):
        j[i] = random.randint(0,l-K)

    iter_infos = [] # scalar of the guess motif's information per iteration
    iter_scores = [] # n-tuple of each sequence's score per iteration
    iter_datas = []

    for x in range(num_iterations):
        guess_motif = []
        guess_data = []
        guess_score = [0] * n

        for i in range(n):     # create new weight matrix
            guess_motif.append(seq[i][j[i]:j[i]+K])
            W = get_W(guess_motif) # why not outside the loop?
            H_M = get_HM(guess_motif)

        for i in range(n):      # get new scores
            guess_score[i] = S(guess_motif[i], W)
            guess_data.append(str(guess_motif[i]))
            guess_data.append(str(guess_score[i])[0:3])
            print str(guess_score[i])[0:3]
            print guess_data
        guess_data.append(str(sum(guess_score))[0:3])

        iter_infos.append(H_bg - sum(H_M)/K)
        iter_scores.append(guess_score)
        iter_datas.append(guess_data)

        print "guess_score: %s" % guess_score
        print "iter_scores: %s" % iter_scores

        for i, s in enumerate(seq):   # update guess motif by weight matrix
            for k in range(l-K): # check if this is right
                if S(s[k], W) < guess_score[i]:
                    guess_score[i] = S(s[i], W)
                    j[i] = k

    infos.append(iter_infos)          # log that iteration's scores/infos/data
    scores.append(iter_scores)
    datas.append(iter_datas)

    print "iter_scores[0]: %s" % iter_scores[0]

best_sum_scores = []

for iterscores in scores:
    sum_scores = []
    for iterscore in iterscores:
        sum_scores.append(sum(iterscore))
    best_sum_scores.append(max(sum_scores))

best_indices = [i for i, j in enumerate(best_sum_scores) if j==max(best_sum_scores)]

for b in best_indices:
    plt.figure()
    info_line, = plt.plot(infos[b], 'r')
    score_line, = plt.plot([sum(s) for s in scores[b]], 'b')
    print "Guess progression for iteration #%s:" % str(b)
    for m in datas[b]:
        print m
    print [str(sum(s)) for s in scores[b]]
    print " "
    plt.xlabel("Iteration")
    plt.ylabel("Score")
    plt.title("Score per iteration in Gibb's Sampling")
    plt.legend([info_line, score_line], ["Information", "Score"])
    plt.show()
