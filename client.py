#!/usr/bin/python
"""
example kwetter client which talks 0MQ directly to kwetterd.
"""

import zmq
import json
from m2client import M2Kwetter

SERVER="tcp://localhost:5555"

class Kwetter(M2Kwetter):
    def __init__(self, server):
        context = zmq.Context()
        self.socket = context.socket(zmq.REQ)
        self.socket.connect(server)

    def write(self, message): 
        self.socket.send(json.dumps(message,separators=(',',':')))
        return self.socket.recv()

if __name__ == '__main__':
    import doctest
    doctest.testmod(optionflags=doctest.ELLIPSIS)
