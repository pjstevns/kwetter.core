#!/usr/bin/python
import zmq

SERVER="tcp://localhost:5555"

class Kwetter(object):
    def __init__(self, server):
        context = zmq.Context()
        self.socket = context.socket(zmq.REQ)
        self.socket.connect(server)

    def write(self, message): return self.socket.send(message)
    def read(self): return self.socket.recv()

    def reg(self, avatar, fullname):
        """
        register a new avatar
        >>> conn = Kwetter(SERVER)
        >>> conn.reg('newname', 'Paul Stevens')
        'OK'
        """
        self.write('/reg/%s/%s' %(avatar, fullname))
        return self.read()

    def unreg(self, avatar):
        """
        remove an avatar from the registry
        """
        self.write('/unreg/%s/%s' %(avatar))
        return self.read()

    def rereg(self, avatar, newavatar, newfullname):
        """
        update the entry of an avatar in the registry
        """
        self.write('/rereg/%s/%s/%s' % (avatar, newavatar, newfullname))
        return self.read()

    def info(self, avatar):
        """
        show fullname, followers and followees for avatar
        """
        self.write('/info/%s' % avatar)
        return self.read()

    def post(self, avatar, message):
        """
        post a new message
        """
        self.write('/post/%s/%s' % (avatar, message))
        return self.read()

    def search(self, avatar, string, limit):
        """
        search for last 'limit' messages containing 'string'
        """
        self.write('/search/%s/%s/%d' % (avatar, string, limit))
        return self.read()

    def follow(self, avatar, otheravatar):
        """
        subscribe to messages from other avatar
        """
        self.write('/follow/%s/%s' % (avatar, otheravatar))
        return self.read()

    def unfollow(self, avatar, otheravatar):
        """
        unsubscribe from messages from other avatar
        """
        self.write('/unfollow/%s/%s' % (avatar, otheravatar))
        return self.read()

    def update(self, avatar, timestamp):
        """
        show all messages of subscribed avatars since timestamp
        """
        self.write('/update/%s/%d' % (avatar, timestamp))
        return self.read()

if __name__ == '__main__':
    import doctest
    doctest.testmod(optionflags=doctest.ELLIPSIS)
