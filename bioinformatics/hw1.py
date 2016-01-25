#! /usr/bin/python2.7

from Bio import SeqIO

seq = SeqIO.parse(open("sequence"), "fasta")
for ref in seq:
    print ref.seq
