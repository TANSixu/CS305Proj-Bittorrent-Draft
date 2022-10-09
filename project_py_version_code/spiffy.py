import socket
import spiffy

class SUSTCSocket(socket.socket):
    def __init__(self) -> None:
        # You can ONLY use UDP!!!! Whoever changes this will be punished!!!!!!!
        super().__init__(socket.AF_INET, socket.SOCK_DGRAM)
    
    def sendto(self, __data,  __flags, __address) -> int:
        print("call spiffy socket!\n")
        ret = spiffy.spiffy_sendto(self.fileno, __data, len(__data), __flags, __address[0], __address[1])
        return ret

    
sock = SUSTCSocket()
sock.sendto("hello",0,("127.0.0.1",3667))