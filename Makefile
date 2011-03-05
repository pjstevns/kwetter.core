
kwetterd: kwetterd.c
	gcc -o kwetterd -Wall -I /usr/include/zdb `pkg-config --cflags --libs libzmq` -lzdb kwetterd.c

clean:
	rm -rf *.o kwetterd
