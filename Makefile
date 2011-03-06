
# base
CFLAGS = -Wall -g -O2

# libjson0
CFLAGS += $(shell pkg-config --cflags json)
LDFLAGS += $(shell pkg-config --libs json)

# libzmq
CFLAGS += $(shell pkg-config --cflags libzmq)
LDFLAGS += $(shell pkg-config --libs libzmq)

# libzdb
CFLAGS += -I /usr/include/zdb
LDFLAGS += -lzdb

kwetterd: kwetterd.c commands.c helpers.c
	gcc -c $(CFLAGS) -o helpers.o helpers.c
	gcc -c $(CFLAGS) -o commands.o commands.c
	gcc -c $(CFLAGS) -o kwetterd.o kwetterd.c
	gcc $(LDFLAGS) -o $@ helpers.o commands.o kwetterd.o

clean:
	rm -rf *.o kwetterd
