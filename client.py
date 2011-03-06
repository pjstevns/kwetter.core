#!/usr/bin/python
import zmq
import json

SERVER="tcp://localhost:5555"

class Kwetter(object):
    def __init__(self, server):
        context = zmq.Context()
        self.socket = context.socket(zmq.REQ)
        self.socket.connect(server)

    def write(self, message): 
        self.socket.send(json.dumps(message,separators=(',',':')))
        return self.socket.recv()

    def reg(self, avatar, fullname):
        """
        register a new avatar
        >>> conn = Kwetter(SERVER)
        >>> conn.reg('newname', 'Paul Stevens')
        'OK'
        """
        return self.write(dict(command='reg',avatar=avatar,fullname=fullname,))

    def unreg(self, avatar):
        """
        remove an avatar from the registry
        >>> conn = Kwetter(SERVER)
        >>> conn.reg('newname', 'Paul Stevens')
        'OK'
        >>> conn.unreg('newname')
        'OK'
        """
        return self.write(dict(command='unreg',avatar=avatar))

    def rereg(self, avatar, newavatar, newfullname):
        """
        update the entry of an avatar in the registry
        >>> conn = Kwetter(SERVER)
        >>> conn.reg('othername', 'Paul Stevens')
        'OK'
        >>> conn.rereg('othername', 'newname', 'Paul J Stevens')
        'OK'
        """
        return self.write(dict(command='rereg',avatar=avatar,
                               newavatar=newavatar, newfullname=newfullname))

    def info(self, avatar):
        """
        show fullname, followers and followees for avatar
        >>> conn = Kwetter(SERVER)
        >>> conn.reg('othername', 'Paul Stevens')
        'OK'
        >>> conn.info('othername')
        '[ "othername", "Paul Stevens" ]'
        """
        return self.write(dict(command='info',avatar=avatar))

    def post(self, avatar, message):
        """
        post a new message
        """
        return self.write(dict(command='post',avatar=avatar,message=message))

    def search(self, avatar, string, since, limit):
        """
        search for last 'limit' messages containing 'string'
        """
        return self.write(dict(command='search', avatar=avatar, string=string,
                               since=since, limit=limit))

    def follow(self, avatar, otheravatar):
        """
        subscribe to messages from other avatar
        """
        return self.write('/follow/%s/%s' % (avatar, otheravatar))

    def unfollow(self, avatar, otheravatar):
        """
        unsubscribe from messages from other avatar
        """
        return self.write('/unfollow/%s/%s' % (avatar, otheravatar))

    def update(self, avatar, timestamp):
        """
        show all messages of subscribed avatars since timestamp
        """
        return self.write('/update/%s/%d' % (avatar, timestamp))

if __name__ == '__main__':
    import doctest
    doctest.testmod(optionflags=doctest.ELLIPSIS)
