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

# avenum = 300
# ave_entropy = np.empty(avenum)
# ave_width = np.floor(human_seqlen/avenum)

# for a in range(avenum):
#     print np.mean(entropy[a*ave_width:(a+1)*ave_width])
#     ave_entropy[a] = np.mean(entropy[a*ave_width:(a+1)*ave_width])

plt.title("Relative entropy of AAA1")
plt.xlabel("base")
plt.ylabel("relative entropy (bits)")
plt.plot(np.linspace(1651,1875,1875-1651), entropy[1651:1875])
plt.show()

stem_entropy = np.mean(entropy[1:1650])
AAA1_entropy = np.mean(entropy[1651:1875])
AAA2_entropy = np.mean(entropy[1938:2161])
AAA3_entropy = np.mean(entropy[2251:2505])
AAA4_entropy = np.mean(entropy[2617:2863])
stalk_entropy = np.mean(entropy[2881:3169])
AAA5_entropy = np.mean(entropy[3244:3473])
AAA6_entropy = np.mean(entropy[3690:3905])

CC1_entropy = np.mean(entropy[1074:1103])
CC2_entropy = np.mean(entropy[2897:2982])
CC3_entropy = np.mean(entropy[3109:3200])
CC4_entropy = np.mean(entropy[3408:3442])

print "stem_entropy: ", stem_entropy 
print "AAA1_entropy: ", AAA1_entropy  
print "AAA2_entropy: ", AAA2_entropy  
print "AAA3_entropy: ", AAA3_entropy  
print "AAA4_entropy: ", AAA4_entropy  
print "stlk_entropy: ", stalk_entropy
print "AAA5_entropy: ", AAA5_entropy  
print "AAA6_entropy: ", AAA6_entropy

print "CC1_entropy: ", CC1_entropy
print "CC2_entropy: ", CC2_entropy
print "CC3_entropy: ", CC3_entropy
print "CC4_entropy: ", CC4_entropy  

sorted_entropy = np.sort(list(set(entropy)))
print sorted_entropy

for rank, entro in enumerate(sorted_entropy):
    print "rank ", rank, " entropy ", entro
    for idx, ent in enumerate(entropy):
        if ent == entro:
            print idx


SRP_seqs = [209, 330, 338, 430, 495, 587, 871 , 1228, 1240, 1379, 1423, 1442, 1537, 1987, 1991, 1991, 2205, 2227,
2304, 2362, 2461, 2481, 2496, 2532, 2555, 2573, 2640, 2662, 2819, 3015, 3381, 3762, 3806, 3847, 4232]

background_entropy = np.mean(entropy[3408:3442])
SRP_entropy = np.mean([entropy[i] for i in SRP_seqs])

print "background entropy: ", background_entropy
print "SRP entropy: ", SRP_entropy
