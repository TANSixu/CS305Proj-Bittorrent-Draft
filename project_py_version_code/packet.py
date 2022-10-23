from turtle import bye
from struct import pack
from struct import *

HEADER_LEN=16
PKT_LEN=1500

class Header:
    def __init__(self, _magic, _type, _header_len, _pktlen, _seq, _ack):
        self.magic = _magic
        self.type = _type
        self.header_len = _header_len
        self.pkt_len = _pktlen
        self.seq = _seq
        self.ack = _ack

    def header_to_byte(self):
        bytes = pack("HHHHII", self.magic, self.type, self.header_len, self.pkt_len, self.seq, self.ack)
        return bytes
    

class Packet:
    def __init__(self, header, data_byte):
        pass

    def pkt_to_byte(self):
        pass