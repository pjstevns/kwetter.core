#!/usr/bin/python
"""
example kwetter client which talks JSON over HTTP to mongrel2
"""

import json
import urllib2
from datetime import datetime, timedelta

SERVER="http://localhost:6767"

class M2Kwetter(object):
    def __init__(self, server):
        self.server = server

    def write(self, message): 
        req = urllib2.Request(url=self.server, 
                              data=json.dumps(message,separators=(',',':')),
                              headers={'Content-Type':'application/json'},
                             )
        rep = urllib2.urlopen(req)
        return rep.read().strip()

    def reg(self, avatar, fullname):
        """
        register a new avatar
        >>> conn = M2Kwetter(SERVER)
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
        >>> conn = M2Kwetter(SERVER)
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
        >>> conn = M2Kwetter(SERVER)
        >>> r = conn.unreg('reregname')
        >>> r = conn.unreg('newregname')
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
        >>> conn = M2Kwetter(SERVER)
        >>> r = conn.unreg('infoname')
        >>> conn.reg('infoname', 'Paul Stevens')
        'OK'
        >>> conn.info('infoname')
        '{ "avatar": "infoname", "fullname": "Paul Stevens" }'
        >>> conn.reg('follows', 'John Doe')
        'OK'
        >>> conn.follow('infoname','follows')
        'OK'
        >>> conn.reg('follows2', 'Mary Doe')
        'OK'
        >>> conn.follow('infoname','follows2')
        'OK'
        >>> conn.reg('follower', 'John Deer')
        'OK'
        >>> conn.follow('follower','infoname')
        'OK'
        >>> conn.reg('follower2', 'Mary Deer')
        'OK'
        >>> conn.follow('follower2','infoname')
        'OK'
        >>> conn.info('infoname')
        '{ "avatar": "infoname", "fullname": "Paul Stevens", "follows": [ "follows", "follows2" ], "followers": [ "follower", "follower2" ] }'
        >>> conn.unreg('infoname')
        'OK'
        >>> conn.unreg('follows')
        'OK'
        >>> conn.unreg('follows2')
        'OK'
        >>> conn.unreg('follower')
        'OK'
        >>> conn.unreg('follower2')
        'OK'
        """
        return self.write(dict(command='info',avatar=avatar))

    def follow(self, avatar, follow):
        """
        subscribe to messages from other avatar
        >>> conn = M2Kwetter(SERVER)
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
        >>> conn = M2Kwetter(SERVER)
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
        >>> conn = M2Kwetter(SERVER)
        >>> r = conn.reg('poster', 'Test User')
        >>> conn.post('poster', 'some funky message')
        'OK'
        >>> conn.unreg('poster')
        'OK'
        """
        return self.write(dict(command='post',avatar=avatar,message=message))

    def search(self, avatar, string=None, since=None, limit=10):
        """
        search for last 'limit' messages containing 'string'
        >>> conn = M2Kwetter(SERVER)
        >>> r = conn.reg('poster', 'Test User')
        >>> conn.post('poster', 'some funky message')
        'OK'
        >>> conn.search('poster', 'funky')
        '{ "avatar": "poster", "string": "funky", ...}'
        >>> conn.unreg('poster')
        'OK'
        """
        if not since: since = datetime.today()+timedelta(days=-7)
        if not string: string=""
        return self.write(dict(command='search', avatar=avatar, string=string,
                               since=str(since), limit=limit))

    def timeline(self, avatar, since=None, limit=10):
        """
        show all messages of self and subscribed avatars since 'since'
        >>> conn = M2Kwetter(SERVER)
        >>> r = conn.reg('poster', 'Test User')
        >>> r = conn.reg('groupie', 'Other User')
        >>> r = conn.follow('groupie', 'poster')
        >>> r = conn.post('poster', 'blah message 1')
        >>> conn.timeline('groupie')
        '{ "avatar": "groupie", ... }'

        >>> r = conn.post('poster', 'blah message 2')
        >>> r = conn.post('poster', 'blah message 3')
        >>> r = conn.post('poster', 'blah message 4')
        >>> r = conn.post('poster', 'blah message 5')
        >>> conn.timeline('groupie')
        '{ "avatar": "groupie", ... }'
        >>> conn.unreg('poster')
        'OK'
        >>> conn.unreg('groupie')
        'OK'
        """
        if not since: since = datetime.today()+timedelta(days=-7)
        return self.write(dict(command='timeline', avatar=avatar,
                               since=str(since),limit=str(limit)))

    def updates(self, avatar, since=None, limit=10):
        """ 
        show messages for an avatar
        >>> conn = M2Kwetter(SERVER)
        >>> r = conn.reg('poster', 'Test User')
        >>> r = conn.reg('groupie', 'Other User')
        >>> r = conn.post('poster', 'blah message 1')
        >>> r = conn.post('poster', 'blah message 2')
        >>> conn.updates('groupie')
        'NO'
        >>> conn.updates('poster')
        '{ "avatar": "poster", ... }'

        """

        if not since: since = datetime.today()+timedelta(days=-7)
        return self.write(dict(command='updates', avatar=avatar,
                               since=str(since),limit=str(limit)))

if __name__ == '__main__':
    import doctest
    print "running..."
    doctest.testmod(optionflags=doctest.ELLIPSIS)
