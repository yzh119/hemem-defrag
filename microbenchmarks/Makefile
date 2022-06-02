HEMEMDIR = $(CURDIR)/../src/

CC = gcc
CFLAGS = -g -Wall -O3
#CFLAGS = -g3 -Wall -O0
INCLUDES = -I../linux/usr/include
LIBS = -lm -lpthread

default: gups-pebs gups-random gups-hotset-move gups-pebs-sparse gups-pebs-dense-sparse gups-pebs-base gups-pebs-sparse-base gups-pebs-dense-sparse-base gups-sparse-oracle gups-sparse-oracle-base

all: gups-pebs gups-random gups-simple gups-lru gups-lru-swap gups-hotset-move gups-pebs-sparse gups-pebs-sparse-base gups-pebs-dense-sparse gups-pebs-dense-sparse-base#gups

gups-random: gups-random.o
	$(CC) $(CFLAGS) $(INCLUDES) -o gups-random gups-random.o zipf.o ../src/timer.o $(LIBS) # -L$(HEMEMDIR) -lhemem

gups-small: gups-small.o
	$(CC) $(CFLAGS) $(INCLUDES) -o gups-small gups-small.o ../src/timer.o $(LIBS)

gups: gups.o
	$(CC) $(CFLAGS) $(INCLUDES) -o gups gups.o zipf.o ../src/timer.o $(LIBS)

gups-pebs: gups.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ gups.o zipf.o $(LIBS) -L$(HEMEMDIR) -lhemem

gups-pebs-base: gups.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ gups.o zipf.o $(LIBS) -L$(HEMEMDIR) -lhemem-base

gups-pebs-sparse: gups-sparse.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ gups-sparse.o zipf.o $(LIBS) -L$(HEMEMDIR) -lhemem

gups-pebs-sparse-base: gups-sparse.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ gups-sparse.o zipf.o $(LIBS) -L$(HEMEMDIR) -lhemem-base

gups-sparse-oracle: gups-sparse-oracle-huge.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ gups-sparse-oracle-huge.o zipf.o  ../src/timer.o $(LIBS) -L$(HEMEMDIR) -D PAGE_SIZE=2097152UL

gups-sparse-oracle-base: gups-sparse-oracle-base.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ gups-sparse-oracle-base.o zipf.o  ../src/timer.o $(LIBS) -L$(HEMEMDIR) -D PAGE_SIZE=4096UL

gups-pebs-dense-sparse: gups-dense-sparse.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ gups-dense-sparse.o zipf.o $(LIBS) -L$(HEMEMDIR) -lhemem

gups-pebs-dense-sparse-base: gups-dense-sparse.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ gups-dense-sparse.o zipf.o $(LIBS) -L$(HEMEMDIR) -lhemem-base

gups-hotset-move: gups-hotset-move.o
	$(CC) $(CFLAGS) $(INCLUDES) -o gups-hotset-move gups-hotset-move.o zipf.o $(LIBS) -L$(HEMEMDIR) -lhemem

gups-simple: gups.o
	$(CC) $(CFLAGS) $(INCLUDES) -o gups-simple gups.o zipf.o $(LIBS) -L$(HEMEMDIR) -lhemem-simple

gups-lru: gups.o
	$(CC) $(CFLAGS) $(INCLUDES) -o gups-lru gups.o zipf.o $(LIBS) -L$(HEMEMDIR) -lhemem-lru

gups-lru-swap: gups.o
	$(CC) $(CFLAGS) $(INCLUDES) -o gups-lru-swap gups.o zipf.o $(LIBS) -L$(HEMEMDIR) -lhemem-lru-swap

gups-random.o: gups-random.c zipf.c gups.h
	$(CC) $(CFLAGS) $(INCLUDES) -c gups-random.c zipf.c

gups.o: gups.c zipf.c  gups.h
	$(CC) $(CFLAGS) $(INCLUDES) -c gups.c zipf.c

gups-sparse.o: gups-sparse.c zipf.c  gups.h
	$(CC) $(CFLAGS) $(INCLUDES) -c gups-sparse.c zipf.c

gups-sparse-oracle-base.o: gups-sparse-oracle.c zipf.c  gups.h
	$(CC) $(CFLAGS) $(INCLUDES) -c gups-sparse-oracle.c -D PAGE_SIZE=4096UL -o $@

gups-sparse-oracle-huge.o: gups-sparse-oracle.c zipf.c  gups.h
	$(CC) $(CFLAGS) $(INCLUDES) -c gups-sparse-oracle.c -D PAGE_SIZE=2097152UL -o $@

gups-dense-sparse.o: gups-dense-sparse.c zipf.c  gups.h
	$(CC) $(CFLAGS) $(INCLUDES) -c gups-dense-sparse.c zipf.c

gups-hotset-move.o: gups-hotset-move.c zipf.c gups.h
	$(CC) $(CFLAGS) $(INCLUDES) -c gups-hotset-move.c zipf.c

clean:
	$(RM) *.o gups gups-hotset-move gups-lru-swap gups-lru gups-simple gups-random gups-pebs
