#! /usr/bin/python2.7

import matplotlib.pyplot as plt
import numpy as np
import sys

def get_entropy(column, text):
    aminoacids = "GPAVLIMCFYWHKRQNEDST"
    entropy = 0
    for a in aminoacids:
        background_p = float(text.count(a)) / len(text)
        freq = float(column.count(a)) / len(column)
        if background_p != 0 and freq != 0:
            entropy += freq * np.log2(freq / background_p)
            #print freq, background_p, freq/background_p, entropy
    return entropy

if len(sys.argv) != 2:
    print "usage: ./script input_file"
    exit(1)

raw_file = open(sys.argv[1], 'r').read()
chunks = raw_file.split('\n\n')

numchunks = len(chunks)
numseqs   = len(chunks[0].split('\n'))

seqs = [""]*numseqs

for c in range(numchunks):
    chunk = chunks[c].split('\n')
    for n in range(numseqs):
        seqs[n] += chunk[n][-60:]

human_position = 7
        
seqlen = len(seqs[0])
human_seqlen = seqlen - seqs[human_position].count('-')

entropy = np.zeros(human_seqlen)
human_idx = 0

background_text = "".join(seqs)

for n in range(seqlen):
    if (seqs[human_position][n] != '-'):
        column = "".join([seq[n] for seq in seqs])
        entropy[human_idx] = get_entropy(column, background_text)
        human_idx += 1
    else:
        continue

avenum = 300
ave_entropy = np.empty(avenum)
ave_width = np.floor(human_seqlen/avenum)

for a in range(avenum):
    print np.mean(entropy[a*ave_width:(a+1)*ave_width])
    ave_entropy[a] = np.mean(entropy[a*ave_width:(a+1)*ave_width])

plt.title("Relative entropy of DYNCH2")
plt.xlabel("base")
plt.ylabel("relative entropy (bits)")
plt.plot(np.linspace(0,human_seqlen,avenum), ave_entropy)
plt.show()
