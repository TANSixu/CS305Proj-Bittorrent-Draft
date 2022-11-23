import logging
from enum import Enum

class State(Enum):
    send_stop_and_wait = 1
    send_fixed_window = 2
    send_slow_start = 3
    send_congestion_avoidance = 4
    send_fast_retransmit = 5
    receiving = 6

class Connection:
    def __init__(self, _rtt) -> None:
        sending_queue = []
        state = State.send_stop_and_wait
        winsize = 8
        ssthresh = 64
        assert _rtt > 0
        rtt = _rtt
        duplicatedACKcnt = 0
        

    def set_state(self, next_state: Enum):
        if next_state is State.send_stop_and_wait:
            self.winsize = 1
            self.state = next_state
            return
        elif next_state is State.send_fixed_window:
            self.winsize = 8
            self.state = next_state
            return
        else: 
            if self.state is State.send_slow_start:
                if next_state is State.send_slow_start:
                    self.winsize = 1
                    self.ssthresh = max(self.winsize//2, 2)
                    self.state = next_state
                elif next_state is State.send_congestion_avoidance:
                    self.state = next_state
                    pass
            elif self.state is State.send_congestion_avoidance:
                if next_state is State.send_slow_start:
                    self.winsize = 1
                    self.ssthresh = max(self.winsize//2, 2)
                    self.state = next_state
        print("error no matching state transit!")

    def send_data():
        