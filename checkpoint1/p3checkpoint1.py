#!/usr/bin/env python

import os, sys, signal
import struct
import socket
from select import select
from time import sleep
from popen2 import Popen3

_magicnum = 15441
_version = 1
_whohas = 0
_ihave = 1
_timeout = 10
_floodfile = 'temp.flood'
_bufsize = 1500
_hostname = socket.gethostname()

def hex2str(hexval):
    bytes = []
    for i in range(0, len(hexval), 2):
        bytes.append( chr( int (hexval[i:i+2], 16 ) ) )
    return ''.join( bytes )

_hash1 = hex2str('3f06e8fce70f85692fc191c417ac3218420941ab')
_hash2 = hex2str('79e0deb50716343adef2b4b2c6f2fa116f046478')

ihave_count = 0
whohas_count = 0
peerlist = []
listensock = None

def send_to_peers(pkt):
    for addr in peerlist:
        if addr != (_hostname, myport):
            listensock.sendto(pkt, addr)
    
def register_client():
    sock = socket.socket()
    sock.connect(trackeraddr)
    sock.send('REGISTER 2 %s %d' % (_hostname, myport))
    recp = sock.recv(_bufsize)
    for line in recp.splitlines():
        try:
            params = line.split()
            nodeid = params[0]
            peeraddr = params[1]
            peerport = int(params[2])
            if (peeraddr, peerport) not in peerlist:
                peerlist.append((peeraddr, peerport))
        except IndexError:
            pass
    sock.close() 

def send_WHOHAS():
    print 'Sending WHOHAS packet...'
    hdr_hdrlen = 16
    hdr_pktlen = 16 + 4 + 2*20
    
    header = struct.pack('!hbbhhII',
                         _magicnum,
                         _version,
                         _whohas,
                         hdr_hdrlen,
                         hdr_pktlen,
                         0,
                         0)
    
    data = struct.pack('bxxx20s20s', 2, _hash1, _hash2)
    packet = header + data
    send_to_peers(packet)

def handle_packet(buf):
    global ihave_count, whohas_count
    magicnum, version, pkttype, hdrlen, pktlen, _, _ = struct.unpack('!hbbhhII', buf[:16])
    if (magicnum == _magicnum and version == 1):
        if pkttype == _ihave:
            print 'Received IHAVE packet!'
            ihave_count += 1
        elif pkttype == _whohas:
            print 'Received WHOHAS packet!'
            whohas_count += 1    

def create_listensock():
    global myport, listensock
    myport = 20000 + teamnum*100 + 57
    listensock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    listensock.bind((socket.gethostname(), myport))

def main():
    print 'Starting tracker...'
    trackerpid = (Popen3('python bf_runtracker.py %d' % trackerport)).pid
    sleep(1)
    print 'Starting client...'
    peerpid = (Popen3('./bitflood %s -n 1' % _floodfile)).pid
    sleep(1)
    try:
        try:
            create_listensock()
            register_client()
        except socket.error:
            print 'ERROR connecting to tracker.'
            return
        
        sleep(1)

        for i in xrange(2):
            send_WHOHAS()
            while ihave_count == 0 or whohas_count == 0:
                readfds, _, _ = select([listensock], [], [], _timeout)
                if listensock in readfds:
                    buf, _ = listensock.recvfrom(_bufsize)
                    handle_packet(buf)
                else:
                    break
    finally:
        os.kill(trackerpid, signal.SIGKILL)
        os.kill(peerpid, signal.SIGKILL)
        
        score = 5
        if ihave_count == 0:
            print 'ERROR: Timeout before received IHAVE packet.'
            score = 0
        if whohas_count == 0:
            print 'ERROR: Timeout before received WHOHAS packet.'
            score = 0

        print 'Team %d score: %d' % (teamnum, score)

if __name__ == '__main__':
    global teamnum, trackerport, trackeraddr
    try:
        teamnum = int(sys.argv[1])
        trackerport = 20000 + 100*teamnum + 56
        trackeraddr = ('localhost', trackerport)
        fp = open(_floodfile, 'w')
        try:
            print >> fp, 'localhost:%d' % trackerport
            print >> fp, 'temp'
            print >> fp, '600001'
            print >> fp, '0 3f06e8fce70f85692fc191c417ac3218420941ab'
            print >> fp, '1 79e0deb50716343adef2b4b2c6f2fa116f046478'
        finally:
            fp.close()
    except IndexError:
        print 'Usage: %s <team-num>' % sys.argv[0]
        sys.exit(-1)
        
    main()
