CXX=g++
CXXFLAGS=-ggdb -Wall -Wextra 
LIBS=-lzmq -ljson -lm2pp

# base
CFLAGS = -Wall -O2 -g

# mongrel2
M2PP_CFLAGS?=
M2PP_LDFLAGS?=

# libjson0
CFLAGS += $(shell pkg-config --cflags json)
LDFLAGS += $(shell pkg-config --libs json)

# libzmq
ZMQ_CFLAGS?=$(shell pkg-config --cflags libzmq)
CFLAGS += $(ZMQ_CFLAGS)
ZMQ_LDFLAGS?=$(shell pkg-config --libs libzmq)
LDFLAGS += $(ZMQ_LDFLAGS)

# libzdb
ZDB_CFLAGS?=-I /usr/include/zdb
CFLAGS += $(ZDB_CFLAGS)
ZDB_LDFLAGS?= -L/usr/local/lib -lzdb 
LDFLAGS += $(ZDB_LDFLAGS)

all: kwetterd kwetter-m2

## main daemon
kwetterd: kwetterd.o commands.o helpers.o
	gcc $(LDFLAGS) -o $@ helpers.o commands.o kwetterd.o

helpers.o: helpers.c
	gcc -c $(CFLAGS) -o helpers.o helpers.c

commands.o: commands.c
	gcc -c $(CFLAGS) -o commands.o commands.c

kwetterd.o: kwetterd.c
	gcc -c $(CFLAGS) -o kwetterd.o kwetterd.c

# mongrel2 handler
kwetter-m2: kwetter-m2.o
	$(CXX) -o $@ $(CXXFLAGS) $(M2PP_CFLAGS) $(M2PP_LDFLAGS) $(LDFLAGS) kwetter-m2.o $(LIBS)

kwetter-m2.o: kwetter-m2.cpp
	$(CXX) $(CXXFLAGS) $(M2PP_CFLAGS) $(ZMQ_CFLAGS) -o $@ -c $<

clean:
	rm -rf *.o kwetterd kwetter-m2
#######################
#######################

.PHONY: clean install
