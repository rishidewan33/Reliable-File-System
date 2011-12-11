C++ = g++
CC = g++
AR = ar
ARFLAGS = ru
RANLIB = ranlib


PARTSRCS_0 = Disk.cc sthread.cc

PARTOBJS_0 := $(PARTSRCS_0:.cc=.o)

PARTSRCS_1 = $(PARTSRCS_0) ADisk.cc

PARTOBJS_1 := $(PARTSRCS_1:.cc=.o)

PARTSRCS_2 = $(PARTSRCS_1) PTree.cc

PARTOBJS_2 := $(PARTSRCS_2:.cc=.o)

PARTSRCS_3 = $(PARTSRCS_2) FlatFS.cc

PARTOBJS_3 := $(PARTSRCS_3:.cc=.o)

PARTSRCS_4 = $(PARTSRCS_3) FS.cc

PARTOBJS_4 := $(PARTSRCS_4:.cc=.o)


OTHERSRCS = testpart0.cc testpart1.cc testpart1a.cc testpart2.cc testpart3.cc testpart4.cc

SRCS = $(COMMONSRCS) $(OTHERSRCS)

LIBS = -lpthread 

CFLAGS = -g -Wall -D_POSIX_THREAD_SEMANTICS 

#all: testpart0 testpart1 testpart2 testpart3 testpart4
all: testpart0 testpart1 testpart1a

%.o: %.cc 
	$(C++) $(CFLAGS) $< -c -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

etags:
	etags *.cc *.c 

testpart0: $(PARTOBJS_0) testpart0.o
	$(CC) -o testpart0 testpart0.o $(PARTOBJS_0) $(LIBS)

testpart1: $(PARTOBJS_1) testpart1.o
	$(CC) -o testpart1 testpart1.o $(PARTOBJS_1) $(LIBS)

testpart1a: $(PARTOBJS_1) testpart1a.o
	$(CC) -o testpart1a testpart1a.o $(PARTOBJS_1) $(LIBS)

testpart2: $(PARTOBJS_2) testpart2.o
	$(CC) -o testpart2 testpart2.o $(PARTOBJS_2) $(LIBS)

testpart3: $(PARTOBJS_3) testpart3.o
	$(CC) -o testpart3 testpart3.o $(PARTOBJS_3) $(LIBS)

testpart4: $(PARTOBJS_4) testpart4.o
	$(CC) -o testpart4 testpart4.o $(PARTOBJS_4) $(LIBS)

simpleGraph:
	echo "Hit return to continue.";
	echo "Done."


clean:
	/bin/rm -f test *.o core *~ 

