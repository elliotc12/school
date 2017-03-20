#!/usr/bin/python3

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import csv
import numpy as np
import os
import sys

### Constants & Classes ###
GLYCINE_BETA_PRESENCE_ERROR = 30
MISSING_CARBON_PENALTY = 20
LINKED_PRESENCE_MISMATCH_PENALTY = 15

PEAK_ERROR_TOLERANCE = 8

AA_list = ["G", "A", "V", "I", "L", "S", "T", "P", "P2", "D", "E", "K", "R", "N", "Q", "M", "C", "Y", "F", "Y", "H"]

class Peak:
    def __init__(self, peak_id, N_shift, H_shift, Cai_shift, Cbi_shift, Cai1_shift, Cbi1_shift, uncertain,
                 hint_i, hint_i1, hint_seq):
        self.peak_id = peak_id
        self.N_shift = N_shift
        self.H_shift = H_shift
        self.Cai_shift = Cai_shift
        self.Cbi_shift = Cbi_shift
        self.Cai1_shift = Cai1_shift
        self.Cbi1_shift = Cbi1_shift
        self.uncertain = uncertain
        self.hint_i = hint_i
        self.hint_i1 = hint_i1
        self.hint_seq = hint_seq
        self.assignment_preference = []
        self.assignment_error = []

def main():
    ### Checking input ###
    if len(sys.argv) != 3:
        print("Error. usage: ./auto-assign.py infile.csv seq.txt")
        exit(1)

    ### Opening/sanitizing CSV ###
    reader = csv.reader(open(sys.argv[1], "r"), delimiter=",")
    parsed_data = [[num if len(num)>0 else "-1.0" for num in row] for row in list(reader)[1:]]
    data = np.array(parsed_data, dtype="float")

    ### Loading peaks into dictionary indexed by peak_id ###
    peaks = {}
    for i, peak in enumerate(data):
        peaks[int(peak[0])] = Peak(peak_id=int(peak[0]), H_shift=peak[1], N_shift=peak[2], Cai_shift=peak[3],
                     Cbi_shift=peak[4], Cai1_shift=peak[5], Cbi1_shift=peak[6], uncertain=peak[7],
                     hint_i=peak[8], hint_i1=peak[9], hint_seq=peak[10])

    ### Opening seq file ### 
    seq = open(sys.argv[2], "r").read()
    if seq[-1] == "\n":
        seq = seq[:-1]

    jared_assignment = ["-"]*len(seq)
    jared_assignment[46] = 2
    jared_assignment[67] = 4
    jared_assignment[6] = 6
    jared_assignment[45] = 14
    jared_assignment[5] = 7
    jared_assignment[64] = 8
    jared_assignment[36] = 1
    jared_assignment[9] = 71
    jared_assignment[10] = 76
    jared_assignment[55] = 19
    jared_assignment[52] = 20
    jared_assignment[65] = 63
    jared_assignment[11] = 69
    jared_assignment[56] = 53
    jared_assignment[43] = 15
    jared_assignment[57] = 27
    jared_assignment[4] = 28
    jared_assignment[27] = 30

    ### Finding where each peak fits best in sequence, load into peak data struct ###
    num_AAs = len(seq)
    for peak in [peaks[key] for key in peaks]:
        peak_errors = np.empty(len(seq))
        for j in range(len(seq)):
            if seq[j] != "M":
                peak_errors[j] = get_assignment_error(peak, seq[j], seq[j-1])
            else:
                peak_errors[j] = get_assignment_error(peak, "M", "-")

        sorted_peaks = np.sort(peak_errors)
        peak.assignment_preference = np.where(np.in1d(peak_errors, sorted_peaks[:5])==1)[0]
        peak.assignment_errors = peak_errors[peak.assignment_preference]
        print(peak.assignment_errors)

        # peak.assignment_preference = np.where(peak_errors<=PEAK_ERROR_TOLERANCE)[0]
        # peak.assignment_error = peak_errors[np.where(peak_errors<=PEAK_ERROR_TOLERANCE)[0]]

    for peak in [peaks[key] for key in peaks]:
        print("Preference: ", peak.assignment_preference)
        print("    Shifts: ", peak.Cai_shift, peak.Cbi_shift)
        print("i-1 Shifts: ", peak.Cai1_shift, peak.Cbi1_shift)
        for i in peak.assignment_preference:
            print(seq[i], end='')
        print("")
        for i in peak.assignment_preference:
            if i > 0:
                print(seq[i-1], end='')
        print("\nError: ", peak.assignment_error)
        print("\n\n\n-------------------------------------")

    ### Try many different assignments, looking for improvements###
    lowest_error = np.inf
    best_assignment = []

    while(True):
        order = np.asarray([peaks[key].peak_id for key in peaks])
        np.random.shuffle(order)
        assignment = ["-"]*len(seq)
        assignment_text = ["-"]*len(seq)
        solution_error = 0

        for i in order:
            t = 0
            if len(peaks[i].assignment_preference) == 0:
                continue
            np.random.shuffle(peaks[i].assignment_preference)
            while (assignment[peaks[i].assignment_preference[t]] != "-"):  # find an unmade assignment
                if t < len(peaks[i].assignment_preference)-1:
                    t += 1
                else:
                    break
            
            # if t < len(peaks[i].assignment_preference) and get_assignment_error(peak, seq(peaks[i].assignment_error[t]), seq(peaks[i].assignment_error[t]-1) < np.inf:
            assignment[peaks[i].assignment_preference[t]] = i
            assignment_text[peaks[i].assignment_preference[t]] = str("peak "+str(i)+":"+str(seq[i-1]))
            # solution_error += peaks[i].assignment_error[t]

        solution_error = get_linking_error(assignment, peaks)
        if solution_error < lowest_error:
            lowest_error = solution_error
            best_assignment = assignment
            print("Error: ", lowest_error)
            print("Assignment: ", best_assignment)
            jared_agreement = 0
            for i in range(len(assignment)):
                if assignment[i] != "-" and assignment[i] == jared_assignment[i]:
                    i += 1
            print("Jared agreement: ", jared_agreement, " / 18")

### Utility functions ###
def get_assignment_error(peak, AA_i, AA_i1):
    peak_Cai = peak.Cai_shift
    peak_Cbi = peak.Cbi_shift
    peak_Cai1 = peak.Cai1_shift
    peak_Cbi1 = peak.Cbi1_shift
    test_Cai = peak_shifts.get(AA_i, [0,0])[0]
    test_Cbi = peak_shifts.get(AA_i, [0,0])[1]
    test_Cai1 = peak_shifts.get(AA_i1, [0,0])[0]
    test_Cbi1 = peak_shifts.get(AA_i1, [0,0])[1]

    #implement this if/when you need it
    # if len(peak.hint_i)>0 and AA_i not in peak.hint_i:  # forbid disobeying a hint
    #     return np.inf

    # if len(peak.hint_i1)>0 and AA_i1 not in peak.hint_i1:
    #     return np.inf

    if AA_i == "M" or AA_i == "P":  # check if matching against a non-HSQC AA
        return np.inf

    error = 0
    if peak_Cai > 0:
        error += np.abs(peak_Cai - test_Cai)

    if AA_i == "G" and peak_Cbi > 0: # penalty for having a Cb when matching w glycine
        error += GLYCINE_BETA_PRESENCE_ERROR
    elif test_Cbi > 0 and peak_Cbi < 0: # penalty for not having a Cb when matching w non-glycine
        error += MISSING_CARBON_PENALTY
    elif peak_Cbi > 0:
        error += np.abs(peak_Cbi - test_Cbi)

    if AA_i1 == "P":                # check if we're matching an i-1 proline
        error_p1 = 0
        error_p2 = 0
        if peak_Cai1 > 0:
            error_p1 += np.abs(peak_Cai1 - peak_shifts["P1"][0])
            error_p2 += np.abs(peak_Cai1 - peak_shifts["P2"][0])
        else:
            error_p1 += MISSING_CARBON_PENALTY
            error_p2 += MISSING_CARBON_PENALTY
        if peak_Cbi1 > 0:
            error_p1 += np.abs(peak_Cbi1 - peak_shifts["P1"][1])
            error_p2 += np.abs(peak_Cbi1 - peak_shifts["P2"][1])
        else:
            error_p1 += MISSING_CARBON_PENALTY
            error_p2 += MISSING_CARBON_PENALTY
        return error + min(error_p1, error_p2)

    if peak_Cai1 > 0:
        error += np.abs(peak_Cai1 - test_Cai1)
    elif AA_i1 == "G" and peak_Cbi1 > 0:
        error += GLYCINE_BETA_PRESENCE_ERROR
    elif test_Cbi1 > 0 and peak_Cbi1 < 0:
        error += MISSING_CARBON_PENALTY
    elif peak_Cbi1 > 0:
        error += np.abs(peak_Cbi1 - test_Cbi1)

    return error

def get_linking_error(assignment, peaks):
    linking_error = 0
    for i in range(len(assignment)-1):
        if assignment[i] == "-" or assignment[i+1] == "-":
            linking_error += LINKED_PRESENCE_MISMATCH_PENALTY
            continue

        this_peak = peaks[assignment[i]]
        next_peak = peaks[assignment[i+1]]

        if this_peak.Cai_shift > 0 and next_peak.Cai1_shift > 0:
            linking_error += np.abs(next_peak.Cai1_shift - this_peak.Cai_shift)**2
        elif this_peak.Cai_shift > 0 and next_peak.Cai1_shift < 0:
            linking_error += LINKED_PRESENCE_MISMATCH_PENALTY
        elif this_peak.Cai_shift < 0 and next_peak.Cai1_shift > 0:
            linking_error += LINKED_PRESENCE_MISMATCH_PENALTY
        elif this_peak.Cai_shift < 0 and next_peak.Cai1_shift < 0:
            linking_error += LINKED_PRESENCE_MISMATCH_PENALTY

        if this_peak.Cbi_shift > 0 and next_peak.Cbi1_shift > 0:
            linking_error += np.abs(next_peak.Cbi1_shift - this_peak.Cbi_shift)**2
        elif this_peak.Cbi_shift > 0 and next_peak.Cbi1_shift < 0:
            linking_error += LINKED_PRESENCE_MISMATCH_PENALTY
        elif this_peak.Cbi_shift < 0 and next_peak.Cbi1_shift > 0:
            linking_error += LINKED_PRESENCE_MISMATCH_PENALTY
        elif this_peak.Cbi_shift < 0 and next_peak.Cbi1_shift < 0:
            linking_error += LINKED_PRESENCE_MISMATCH_PENALTY
    return linking_error
            
peak_shifts = {
 "G": [43.5,	-1.0],
 "A": [50.8,	17.7],
 "V": [60.7,	30.8],
 "I": [59.6,	36.9],
 "L": [53.6,	40.5],
 "S": [56.6,	62.3],
 "T": [60.2,	68.3],
 "P1":[61.6,	30.6],
 "P2":[61.3,	33.1],
 "D": [52.7,	39.8],
 "E": [54.9,	28.9],
 "K": [54.5,	27.5],
 "R": [54.6,	28.8],
 "N": [51.5,	37.7],
 "Q": [54.1,	28.1],
 "M": [53.9,	31  ],
 "C": [57.9,	26  ],
 "Y": [56.7,	27.4],
 "F": [57.4,	37  ],
 "Y": [57.4,	37  ],
 "H": [53.7,	28  ]
}

### Main ###
if __name__ == '__main__':
    main()
