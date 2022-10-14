from pydoc import importfile
import socket
import Spiffy
import ctypes
# import os

# file = './spiffy.so'
# mod = ctypes.cdll.LoadLibrary(file)

# spiffy_sento = mod.spiffy_sendto
# spiffy_sento.argtypes = 


class SUSTCSocket(socket.socket):
    def __init__(self) -> None:
        # You can ONLY use UDP!!!! Whoever changes this will be punished!!!!!!!
        super().__init__(socket.AF_INET, socket.SOCK_DGRAM)
    
    def sendto(self, __data,  __flags, __address) -> int:
        print("call spiffy socket!\n")
        addr = ctypes.create_string_buffer(20)
        addr.value = __address[0].encode()
        ret = Spiffy.spiffy_sendto(self.fileno(), __data, len(__data), __flags, addr, __address[1])
        return ret

    
sock = SUSTCSocket()
sock.sendto("hello",0,("127.0.0.1",3667))