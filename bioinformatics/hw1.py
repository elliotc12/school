#! /usr/bin/python2.7

from biopython import SeqIO

seq = SeqIO.parse(open("sequence"), "fasta")
for ref in seq:
    print ref.seq
