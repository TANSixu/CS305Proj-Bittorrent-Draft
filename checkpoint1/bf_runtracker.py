#!/usr/bin/env python

import os
import sys
import socket
from time import time, strftime
from select import select

clientids = {}
verbose = False

_defportnum = 8801
_defaddr = '0.0.0.0'
_bufsize = 65536
_queuenum = 10
_timeout = 60

def peerlist():
    return '\r\n'.join(sorted([s for s, _ in clientids.values()])) + '\r\n'

def purgelist():
    todelete = []
    for client in clientids:
        _, lastaccess = clientids[client]
        if lastaccess + _timeout < time():
            todelete.append(client)
            if verbose: print 'Client %s timed out, removing from table.' % client
    for client in todelete:
        del(clientids[client])

def main():
    listensock = socket.socket()
    listensock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listensock.bind((_defaddr, portnum))
    listensock.listen(_queuenum)

    if verbose: print 'Listening for connections on %s:%d' % (_defaddr, portnum)

    while True:
        purgelist()
        readyset, _, _ = select([listensock], [], [], _timeout)        
        for sock in readyset:
            if sock is listensock:
                try:
                    sock, (newaddr, newport) = listensock.accept()
                    sock.settimeout(0.1)
                    buf = sock.recv(_bufsize)
                         
                    requests = buf.splitlines()
                    for request in requests:
                        
                        if request.strip().startswith('REGISTER'):
                            try:
                                params = request.split()
                                nodeid, addr, port = params[1], params[2], params[3]

                                if nodeid not in clientids and verbose:
                                    print '%s: Registering node %s: %s %s' % (strftime('%H:%M:%S'), nodeid, addr, port)

                                clientids[nodeid] = ('%s %s %s' % (nodeid, addr, port), time())
                                
                                sock.send('OK\r\n' + peerlist())
                                sock.close()

                            except IndexError, ValueError:
                                pass
                except socket.error:
                    pass
        
if __name__ == '__main__':
    if '--verbose' in sys.argv:
        verbose = True
    portnum = _defportnum

    try: portnum = int(sys.argv[1])
    except Exception: pass
    
    try: main()
    except KeyboardInterrupt: pass
