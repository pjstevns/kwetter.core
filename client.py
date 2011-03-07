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
        >>> r = conn.unreg('regname')
        >>> conn.reg('regname', 'Paul Stevens')
        'OK'
        >>> conn.unreg('regname')
        'OK'
        """
        return self.write(dict(command='reg',avatar=avatar,fullname=fullname,))

    def unreg(self, avatar):
        """
        remove an avatar from the registry
        >>> conn = Kwetter(SERVER)
        >>> r = conn.unreg('unregname')
        >>> conn.reg('unregname', 'Paul Stevens')
        'OK'
        >>> conn.unreg('unregname')
        'OK'
        """
        return self.write(dict(command='unreg',avatar=avatar))

    def rereg(self, avatar, newavatar, newfullname):
        """
        update the entry of an avatar in the registry
        >>> conn = Kwetter(SERVER)
        >>> r = conn.unreg('reregname')
        >>> conn.reg('reregname', 'Paul Stevens')
        'OK'
        >>> conn.rereg('reregname', 'newreregname', 'Paul J Stevens')
        'OK'
        >>> conn.unreg('newreregname')
        'OK'
        """
        return self.write(dict(command='rereg',avatar=avatar,
                               newavatar=newavatar, newfullname=newfullname))

    def info(self, avatar):
        """
        show fullname, followers and followees for avatar
        >>> conn = Kwetter(SERVER)
        >>> r = conn.unreg('infoname')
        >>> conn.reg('infoname', 'Paul Stevens')
        'OK'
        >>> conn.info('infoname')
        '{ "avatar": "infoname", "fullname": "Paul Stevens" }'
        >>> conn.reg('follows', 'John Doe')
        'OK'
        >>> conn.follow('infoname','follows')
        'OK'
        >>> conn.info('infoname')
        '{ "avatar": "infoname", "fullname": "Paul Stevens", "follows": [ "follows" ] }'
        >>> conn.unreg('infoname')
        'OK'
        >>> conn.unreg('follows')
        'OK'
        """
        return self.write(dict(command='info',avatar=avatar))

    def follow(self, avatar, follow):
        """
        subscribe to messages from other avatar
        >>> conn = Kwetter(SERVER)
        >>> r = conn.unreg('follower')
        >>> r = conn.unreg('followee')
        >>> conn.reg('follower','Joe')
        'OK'
        >>> conn.reg('followee','Jane')
        'OK'
        >>> conn.follow('follower','followee')
        'OK'
        >>> conn.info('follower')
        '{ "avatar": "follower", "fullname": "Joe", "follows": [ "followee" ] }'
        >>> conn.unreg('follower')
        'OK'
        >>> conn.unreg('followee')
        'OK'
        """
        return self.write(dict(command='follow', avatar=avatar, follow=follow))

    def unfollow(self, avatar, follow):
        """
        unsubscribe from messages from other avatar
        >>> conn = Kwetter(SERVER)
        >>> r = conn.unreg('follower')
        >>> r = conn.unreg('followee')
        >>> conn.reg('follower','Joe')
        'OK'
        >>> conn.reg('followee','Jane')
        'OK'
        >>> conn.follow('follower','followee')
        'OK'
        >>> conn.info('follower')
        '{ "avatar": "follower", "fullname": "Joe", "follows": [ "followee" ] }'
        >>> conn.unfollow('follower', 'followee')
        'OK'
        >>> conn.info('follower')
        '{ "avatar": "follower", "fullname": "Joe" }'
        >>> conn.unreg('follower')
        'OK'
        >>> conn.unreg('followee')
        'OK'
        """
        return self.write(dict(command='unfollow', avatar=avatar, follow=follow))

    def post(self, avatar, message):
        """
        post a new message
        >>> conn = Kwetter(SERVER)
        >>> r = conn.reg('poster', 'Test User')
        >>> conn.post('poster', 'some funky message')
        'OK'
        """
        return self.write(dict(command='post',avatar=avatar,message=message))

    def search(self, avatar, string, since, limit):
        """
        search for last 'limit' messages containing 'string'
        """
        return self.write(dict(command='search', avatar=avatar, string=string,
                               since=since, limit=limit))

    def timeline(self, avatar, timestamp):
        """
        show all messages of subscribed avatars since timestamp
        """
        return self.write(dict(command='timeline', avatar=avatar, timestamp=timestamp))

if __name__ == '__main__':
    import doctest
    doctest.testmod(optionflags=doctest.ELLIPSIS)
