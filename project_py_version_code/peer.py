import select
import sys
import simsocket
import argparse
import bt_utils

HOST = '127.0.0.1'
PORT = 38324

def main(args):
    config = bt_utils.BtConfig(args)

    # TODO: TESTING--begin
    if False:
        config.identity = 1
        config.chunk_file = 'chunkfile'
        config.has_chunk_file = 'haschunks'
    # TESTING--end

    # TODO: DEBUG--begin
    if False:
        bt_utils.bt_dump_config(config)
    # DEBUG--end

    peer_run(config)

def process_get(chunkfile, outputfile):
    print('PROCESS GET SKELETON CODE CALLED.  Fill me in! I\'ve been doing! (', chunkfile, ',     ', outputfile, ')')

def process_user_input():
    command, chunkf, outf = input().split(' ')
    if command == 'GET':
        process_get(chunkf, outf)
    else:
        pass

def process_inbound_udp(sock):
    pass

def peer_run(config):
    addr = (config.ip, config.port)
    sock = simsocket.SimSocket(addr)

    # TODO: spiffy_init()

    ready = select.select([sock, sys.stdin],[],[])

    while True:
        read_ready = ready[0]
        if len(read_ready) > 0:
            if sock in read_ready:
                process_inbound_udp(sock)

            if sys.stdin in read_ready:
                print('Waiting for command input...')
                process_user_input()
                

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', type=str, help='<peerfile>     The list of all peers', default='nodes.map')
    parser.add_argument('-c', type=str, help='<chunkfile>    The list of chunks')
    parser.add_argument('-m', type=int, help='<maxconn>      Max # of downloads')
    parser.add_argument('-f', type=str, help='<master-chunk> The master chunk file')
    parser.add_argument('-i', type=int, help='<identity>     Which peer # am I?')
    parser.add_argument('-d', type=int, help='debug level', default=0)
    args = parser.parse_args()

    main(args)
