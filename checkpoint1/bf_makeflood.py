#!/usr/bin/env python

import sha, sys

_piecelen = 512 * 1024

def hashvalue(string):
    return sha.new(string).hexdigest()

def printusage():
    print 'Usage: %s <file> <tracker address>'

def makeflood(filename):    
    fp = open(filename, 'r')
    outfile = open(filename + '.flood', 'w')
    try:
        buf = fp.read()
        print >> outfile, trackeraddr
        print >> outfile, filename
        print >> outfile, len(buf)
        for i in range(0, len(buf), _piecelen):
            print >> outfile, i/_piecelen, hashvalue(buf[i:i+_piecelen])
    finally:
        fp.close()

if __name__ == '__main__':
    try:
        filename = sys.argv[1]
        trackeraddr = sys.argv[2]
    except IndexError:
        printusage()
        sys.exit(-1)
    makeflood(filename)
