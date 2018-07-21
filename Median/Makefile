CC = g++
LD = g++
CCFLAGS = -c -g
LDFLAGS = -lm -lpng -lz -L/usr/local/lib
BASE = image.o

all:	main

main:	main.o $(BASE)
	$(LD) main.o $(BASE) $(LDFLAGS) -o main

clean:
	rm  $(BASE) main.o main


.cc.o:
	$(CC) $(CCFLAGS) $<
