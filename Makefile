CC=gcc
CFLAGS=-g --std=c99
OBJS=mfs.o

all: mfs

mfs: $(OBJS)
	$(CC) -o mfs $(OBJS) $(CFLAGS)

mfs.o: mfs.c
	$(CC) -c -o mfs.o mfs.c $(CFLAGS)

clean:
	rm -f $(OBJS) mfs
