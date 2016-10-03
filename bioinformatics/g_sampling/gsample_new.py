#! /usr/bin/python2.7

from __future__ import division

import numpy as np
import matplotlib.pyplot as plt
import random

## Init code ##
data = np.genfromtxt('seq.txt', dtype='string')
K = int(data[0]) # Kmer length
seqs = data[1:]  # Our sequences
seq_length = len(seqs[0])
num_seqs = len(seqs)
eps = 0.0001     # Pseudocount factor

P_bg = {} # BG probabilities
P_bg['A']= sum([seq.count('A') for seq in seqs]) / (num_seqs*seq_length)
P_bg['T']= sum([seq.count('T') for seq in seqs]) / (num_seqs*seq_length)
P_bg['G']= sum([seq.count('G') for seq in seqs]) / (num_seqs*seq_length)
P_bg['C']= sum([seq.count('C') for seq in seqs]) / (num_seqs*seq_length)

## Helper functions ##

def get_kmer_score(start_idx, seq_idx, W):
    score = 0
    for i in range(K):
        score += (seqs[seq_idx][start_idx + i] == 'A') * W[i]['A']
        score += (seqs[seq_idx][start_idx + i] == 'T') * W[i]['T']
        score += (seqs[seq_idx][start_idx + i] == 'G') * W[i]['G']
        score += (seqs[seq_idx][start_idx + i] == 'C') * W[i]['C']
    return score

def get_weight_matrix(motif_guess):
    W = [{'A':0, 'C':0, 'T':0, 'G':0} for k in range(K)]
    for i in range(K):
        a_count = 0
        g_count = 0
        c_count = 0
        t_count = 0
        for j in range(num_seqs):
            if   seqs[j][motif_guess[j]+i] == 'A': a_count += 1
            elif seqs[j][motif_guess[j]+i] == 'T': t_count += 1
            elif seqs[j][motif_guess[j]+i] == 'G': g_count += 1
            elif seqs[j][motif_guess[j]+i] == 'C': c_count += 1
        W[i]['A'] = np.log2((a_count + eps) / num_seqs / P_bg['A'])
        W[i]['C'] = np.log2((c_count + eps) / num_seqs / P_bg['C'])
        W[i]['G'] = np.log2((g_count + eps) / num_seqs / P_bg['G'])
        W[i]['T'] = np.log2((t_count + eps) / num_seqs / P_bg['T'])
    return W

def get_total_score(motif_guess, W):
    return sum([get_kmer_score(motif_guess[j], j, W) for j in range(num_seqs)])

def get_random_motif_guess():
    motif_guess = np.empty([num_seqs]).astype(int)
    for i in range(num_seqs):
        motif_guess[i] = random.randint(0, seq_length-K)
    return motif_guess

def update_motif(motif_guess):
    W = get_weight_matrix(motif_guess)
    S = [get_kmer_score(motif_guess[s], s, W) for s in range(num_seqs)]
    for j in range(num_seqs):
        score_j = get_kmer_score(motif_guess[j], j, W)
        for new_start in range(seq_length - K):
            if get_kmer_score(new_start, j, W) > score_j:
                score_j = get_kmer_score(new_start, j, W)
                motif_guess[j] = new_start
    return motif_guess

def get_information_content(motif_guess):
    I_sum = 0

    Hbg =  -P_bg['A']*np.log2(P_bg['A']) - P_bg['G']*np.log2(P_bg['G'])
    Hbg += -P_bg['T']*np.log2(P_bg['T']) - P_bg['C']*np.log2(P_bg['C'])

    for i in range(K):
        a_count = 0
        g_count = 0
        c_count = 0
        t_count = 0
        for j in range(num_seqs):
            if   seqs[j][motif_guess[j]+i] == 'A': a_count += 1
            elif seqs[j][motif_guess[j]+i] == 'T': t_count += 1
            elif seqs[j][motif_guess[j]+i] == 'G': g_count += 1
            elif seqs[j][motif_guess[j]+i] == 'C': c_count += 1
        Hm  = -a_count/num_seqs*np.log2((a_count + eps)/num_seqs)
        Hm += -g_count/num_seqs*np.log2((g_count + eps)/num_seqs)
        Hm += -c_count/num_seqs*np.log2((c_count + eps)/num_seqs)
        Hm += -t_count/num_seqs*np.log2((t_count + eps)/num_seqs)
        I_sum += Hbg - Hm
    return I_sum

def make_score_plots():
    plt.figure("Score")
    plt.figure("IC")
    lines = 5
    X = [1, 2, 3, 4, 5]
    for l in range(lines):
        scores = []
        ICs = []
        motif_guess = get_random_motif_guess()
        for i in range(5):
            W = get_weight_matrix(motif_guess)
            motif_guess = update_motif(motif_guess)
            scores.append(get_total_score(motif_guess, W))
            ICs.append(get_information_content(motif_guess))
        plt.figure("Score")
        plt.plot(X, scores)
        plt.figure("IC")
        plt.plot(X, ICs)
    plt.figure("Score")
    plt.title("Score progression during Gibbs sampling")
    plt.xlabel("Iteration")
    plt.ylabel("Score")

    plt.figure("IC")
    plt.title("Info content progression during Gibbs sampling")
    plt.xlabel("Iteration")
    plt.ylabel("Information Content")

    plt.show()

## Main loop ##
def main():
    make_score_plots()

if __name__ == "__main__":
    main()
