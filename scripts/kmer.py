#!/usr/bin/python

#
# This script provides functionality for k-mer
#

import os
import sys

class KMer:
    
    # Name of the K-mer
    name = None
    
    # The actual sequence
    seq = None

class PSL:
    
    # Name of the query
    qname = None
    
    # Name of the target
    tname = None
    
    # Start of the query
    qstart = None
    
    # End of the query
    qend = None

class Break:
    
    # Name of the chromosome
    name = None
    
    b1 = -1    
    b2 = -1

def readKMers(file):
    kmers = []
    with open(file) as f:
        while True:
            l = f.readline()
            if (not l):
                break
            if l[0] == '>':
                kmer = KMer()
                kmer.name = l[1:].strip() 
                kmer.seq  = '????'
                kmers.append(kmer)
    return kmers

def readPSL(file):    
    psls = []
    with open(file) as f:
        while True:
            l = f.readline()
            if (not l):
                break
            if l[0] == '3' and l[1] == '1':
                toks = l.split()
                
                p = PSL()

                p.qname  = toks[9]
                p.tname  = toks[13]                
                p.qstart = int(toks[15])
                p.qend   = int(toks[16])
                
                psls.append(p)
    return psls

if __name__ == '__main__':
    
    # Reference annotation (reference breakpoints)
    b_path = '/Users/tedwong/Sources/QA/data/FusQuin/AFU006.v032.bed'

    # Where the sequence files are
    r_path = '/Users/tedwong/Sources/QA/data/FusQuin/AFU007.v013.fa'
    
    # Where the k-mers are
    k_path = '/Users/tedwong/Sources/QA/K'

    #
    # 1. Read in the reference break points
    #

    breaks = {}

    with open(b_path) as f:
        while True:
            l = f.readline()
            if (not l):
                break
            toks = l.split()

            b = Break()
            
            b.name = toks[3]
            b.b1   = int(toks[1])
            b.b2   = int(toks[2])
    
            breaks[b.name] = b

    out = open('kmer.stats', 'w')

    #
    # Read in the k-mers, we'll need to do blat to generate the location in which the k-mers span across.
    # Assume that the k-mers have been generated by the C++ program.
    #
    #  Eg: blat /Users/tedwong/Sources/QA/data/FusQuin/AFU007.v013.fa NG1_12_P2 output.psl
    #
    # We'll find the spanning kmers for only and only this sequin
    #

    for seq in os.listdir(k_path):
        
        if (seq[0] != 'N'):
            continue

        print ('Analyzing ' + seq)
            
        os.system('rm -rf output.psl')
        os.system('blat ' + 'CTR001.v021.fa' + ' ' + k_path + '/' + seq + ' output.psl')

        # Now read the PSL file
        psls = readPSL('output.psl')

        #
        # Now we know where each of the K-mers are mapped to. We're only interested in the k-mers
        # crossing the fusion point.
        #
        
        # All the k-mers for the sequin
        kmers = readKMers(k_path + '/' + seq)

        for kmer in kmers:
            
            found = False
            
            # What's the PSL alignment for this k-mer?
            for psl in psls:

                # Is this what we want?
                #if psl.qname == kmer and psl.tname == seq:
                if psl.qname == kmer.name:
                                        
                    # What's the known breakpoint for the sequin?
                    b = breaks[seq]
                    
                    # For simplicity, only the first breakpoint is considered... 
                    if psl.qstart <= b.b1 and psl.qend >= b.b1:
                        #out.write(kmer.name + '\t' + kmer.seq + '\t' + seq + '\n')
                        pass

                    found = True                    
                    #break

            if found == False:
                out.write(kmer.name + '\t' + kmer.seq + '\t' + seq + '\n')                
                #print ('Warning: PSL not found for: ' + kmer.name)
            #    pass

    out.close()
