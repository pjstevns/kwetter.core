
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

kwetterd: kwetterd.c
	gcc $(CFLAGS) $(LDFLAGS) -o kwetterd kwetterd.c

clean:
	rm -rf *.o kwetterd
