
kwetterd: a lightweight high-performance micro-blogging engine
==============================================================

license: GPLv3

copyright: Paul Stevens, 2011, NFG Net Facilities Group BV, paul@nfg.nl

features:
---------

  - high-performance architecture leveraging some of the coolest 
    technologies: 0MQ, ZDB, JSON
    - `0MQ low-latency network messaging library <http://zeromq.org>`
    - `ZDB database connection pool <http://www.tildeslash.com/libzdb>`
    - `JSON-C json implementation <http://oss.metaparadigm.com/json-c/>`

aims:
-----

  - lightweight: no bloat, no security. The client protocol is the 0MQ protocol. See
    client.py for example code. Implementing security and access restrictions is up
    to the client.
  - scalable: using 0MQ makes horizontal scalability trivial. Currently the client
    protocol uses the REP/REQ pattern so using fan-out/fan-in patterns
    will allow truly massive horizontal scaling.

motivation:
-----------

I needed a small and simple micro-blogging engine for a project. Seemed like 
a great project to get my hands dirty on 0MQ, arguably the coolest networking
shite since tcp sockets.

easy installation:
------------------

  - use the scripted installer provided at `kwetter <http://github.com/pjstevns/kwetter>`_.

manual installation:
--------------------

  - like libzdb supports MySQL, PostgreSQL, Oracle, and SQLite as well. For now no Oracle 
    schema is included.
  - install libzdb, json-c, zeromq
  - install database server of choice (sqlite3, mysql-server, postgresql)
  - create database:

    mysql::

	mysqladmin create kwetter
	mysql -e "grant all on kwetter.* to kwetter@localhost identified by 'kwetter'"
	mysql kwetter < sql/db.mysql

    sqlite::

        sqlite3 ~/kwetter.db < sql/db.sqlite

    postgresql::

        echo "create user kwetter with nocreatedb nocreaterole nocreateuser encrypted password 'kwetter'"|psql template1
	echo "create database kwetter with owner=kwetter encoding='utf-8'"|psql template1
	psql kwetter < sql/db.postgresql
    
  sudo apt-get install mysql-server libzdb-dev libjson0-dev libzeromq-dev
  FIXME: libzdb is not yet available on debian/ubuntu. Packages can be gotten by adding
  to your sources.list:

  deb http://debian.nfgd.net/debian unstable main

  - compile
  
  make
  - edit the config file kwetter.conf and make sure the database URL is correct
  - and run
  ./kwetterd kwetter.conf

  - test
  sudo easy_install pyzmq
  python ./client.py

