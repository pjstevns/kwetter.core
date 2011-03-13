
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
LDFLAGS += -lzdb

kwetterd: kwetterd.o commands.o helpers.o
	gcc $(LDFLAGS) -o $@ helpers.o commands.o kwetterd.o
	##strip $@

helpers.o: helpers.c
	gcc -c $(CFLAGS) -o helpers.o helpers.c

commands.o: commands.c
	gcc -c $(CFLAGS) -o commands.o commands.c

kwetterd.o: kwetterd.c
	gcc -c $(CFLAGS) -o kwetterd.o kwetterd.c

clean:
	rm -rf *.o kwetterd
