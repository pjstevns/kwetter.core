CXX=g++
CXXFLAGS=-ggdb -Wall -Wextra 
LIBS=-lzmq -ljson -lm2pp

# base
CFLAGS = -Wall -O2 -g

# libjson0
CFLAGS += $(shell pkg-config --cflags json)
LDFLAGS += $(shell pkg-config --libs json)

# libzmq
CFLAGS += $(shell pkg-config --cflags libzmq)
LDFLAGS += $(shell pkg-config --libs libzmq)

# libzdb
CFLAGS += -I /usr/include/zdb
LDFLAGS += -L/usr/local/lib -lzdb 

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
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) kwetter-m2.o $(LIBS)

kwetter-m2.o: kwetter-m2.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -rf *.o kwetterd kwetter-m2
#######################
#######################

.PHONY: clean install
