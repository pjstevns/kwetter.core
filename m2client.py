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

    def tag(self, avatar, message, name, value):
        """
        assign attributes to messages
        >>> conn = M2Kwetter(SERVER)
        >>> r = conn.reg('tagger', 'Test User')
        >>> r = conn.reg('tagviewer', 'Other User')
        >>> r = conn.follow('tagviewer', 'tagger')
        >>> r = conn.post('tagger', 'blah message 1')
        >>> conn.timeline('tagviewer')
        '{ "avatar": "tagviewer", ... }'
        >>> r = conn.post('tagger', 'blah message 2')
        >>> r = conn.post('tagger', 'blah message 3')
        >>> r = conn.post('tagger', 'blah message 4')
        >>> r = conn.post('tagger', 'blah message 5')
        >>> r = conn.timeline('tagviewer')
        >>> m = json.loads(r)
        >>> id = m.get('messages')[0][0]
        >>> int(id) > 0
        True
        >>> conn.tag('tagger', id, 'tag', 'test')
        'OK'
        >>> r = conn.timeline('tagviewer')
        >>> m = json.loads(r)
        >>> message = [ x for x in m.get('messages') if x[0] == id ][0]
        >>> message[0] == id
        True

        """
        return self.write(dict(command='tag', avatar=avatar,
                               message=str(message),name=name,value=value ))

    def untag(self, avatar, message, name, value):
        """
        un-assign attributes to messages
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
        >>> r = conn.timeline('groupie')
        >>> m = json.loads(r)
        >>> id = m.get('messages')[0][0]
        >>> conn.tag('poster', id, 'testtag', 'testvalue')
        'OK'
        >>> conn.untag('poster', id, 'testtag', 'testvalue')
        'OK'
        >>> r = conn.timeline('groupie')
        >>> m = json.loads(r)
        >>> messages = [ x for x in m.get('messages') if x[0] == id ]
        >>> messages[0][0] == id
        True
        >>> conn.unreg('poster')
        'OK'
        >>> conn.unreg('groupie')
        'OK'
        """
        return self.write(dict(command='untag', avatar=avatar,
                               message=str(message),name=name,
                               value=value))

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
    doctest.testmod(optionflags=doctest.ELLIPSIS)
